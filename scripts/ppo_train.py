"""PPO 训练主循环：FirstLight 野猪专精（迁移到 yesdelay，下牌 20 tick 生效）在离线引擎上继续训练。

每次更新：顺序打 K 局（学习方随机分边，用野猪 2.6），对手从对手池抽：
  - rules（20%）：规则 bot，野猪 2.6 镜像；
  - everlight（40%）：FirstLight 通用模型（按 20 tick 延迟训练），用卡组池里的真实天梯卡组；
  - snapshot（40%）：学习方的初始模型（冻结），一半用卡组池卡组、一半野猪镜像。
然后用 FirstLight 的 PPO 更新（单进程），保存模型，写日志。

防"钻空子"监控（视频里踩过的坑）：每局记录对手出牌数；对手整局出牌 ≤ 3 的局单独统计。

用法（实例 1 跑离线引擎）：
  $env:CRAI_FL_ROOT="yesdelay"; $env:CRAI_INSTANCE="1"
  .venv\\Scripts\\python.exe -u scripts/ppo_train.py [更新次数 1] [每次局数 4] [输出目录 models/ppo/run1] [起点模型]
断点续跑：输出目录里有 latest.pt 时从它继续（含优化器状态）。
"""
import json
import os
import random
import sys
import time
from pathlib import Path

os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))

import torch  # noqa: E402

from crai.data import cards  # noqa: E402
from crai.env.fl_agent import FLAgent, load_model  # noqa: E402
from crai.env.match import HOG26_SPEC, DeckSpec, play_match  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402
from crai.rl.traj_pack import pack_steps, unpack_steps  # noqa: E402
from crai.rl.ppo_fl import (  # noqa: E402
    Episode, PPOLearner, RewardConfig, build_rollout, episode_rewards, ppo_update, timestamp,
)
from native_runner.training.v4.checkpoint import save_actor_critic_checkpoint  # noqa: E402
from native_runner.training.v4.imitation import ILLossWeightsV4  # noqa: E402
from native_runner.training.v4.ppo_expert_bc import distributed_expert_bc_update_v4  # noqa: E402

BC_COEF = float(os.environ.get("CRAI_BC_COEF", "0.2"))      # 人类回放行为克隆系数，0 关闭


class _ReuseSampler:
    """同一次更新里多步 BC 复用同一批人类局（构建序列很慢，每局约 8 秒）。"""

    def __init__(self, inner):
        self.inner, self.items = inner, None

    def sample(self, update_step, rank, world):
        if self.items is None:
            self.items = self.inner.sample(update_step, rank, world)
        return self.items
from native_runner.training.v4.policy_session import load_policy_v4  # noqa: E402

INIT = ROOT / "models/ppo/hog2_yesdelay_init.pt"
POOL = ROOT / "data/ppo/deck_pool.json"
# 对手池（2026-11-23 改为联赛式）：run2 里 Everlight 几乎全赢、规则 bot 几乎全输，奖励信号几乎恒定，训练退化。
# league：输出目录里学习方自己的历史检查点（采样出牌，和学习方同温度），一半取最近 LEAGUE_RECENT 个、一半取全部历史；
# snapshot：冻结起点（也采样）；everlight / rules 少量保留，防遗忘。可用 CRAI_OPPONENTS="league:0.6,snapshot:0.15,..." 覆盖。
OPPONENTS = [(k, float(v)) for k, v in (x.split(":") for x in os.environ.get(
    "CRAI_OPPONENTS", "league:0.6,snapshot:0.15,everlight:0.15,rules:0.1").split(","))]
LEAGUE_RECENT = int(os.environ.get("CRAI_LEAGUE_RECENT", "5"))
OPP_SAMPLE = os.environ.get("CRAI_OPP_SAMPLE", "1") == "1"     # 模型对手是否采样出牌（与学习方对称）
EXTRA = {"delay_policy": "earliest", "inference_delay_offset_max_ms": 200}   # 起点检查点自带 extra 时以它为准


def forms_of(spec: DeckSpec) -> dict[int, int]:
    return {cards.card_id(k): v for k, v in spec.forms.items()}


def load_pool() -> list[DeckSpec]:
    if not POOL.exists():
        return [HOG26_SPEC]
    from crai.env.fl_agent import _bundle
    known = {int(c) for c in _bundle().card_specs}        # FirstLight 模型认识的卡（16 版新卡不在里面）
    out, weights = [], []

    def add(path: Path, share: float | None):
        """share=None 按出现次数；share=x 把这一池的总权重缩放成已有总权重的 x 倍。"""
        if not path.exists():
            return
        ds = [d for d in json.loads(path.read_text(encoding="utf-8"))
              if all(s in cards.CARDS and cards.card_id(s) in known for s in d["deck"])]
        if not ds:
            return
        scale = 1.0 if share is None or not weights else share * sum(weights) / sum(d["count"] for d in ds)
        for d in ds:
            out.append(DeckSpec(d["deck"], d["forms"], tower=d["tower"]))
            weights.append(d["count"] * scale)

    add(POOL, None)                                                   # HF：野猪局的对手卡组（真实分布）
    add(ROOT / "data/royaleapi/top200/deck_pool.json", 1.0)           # Top200 玩家对局的卡组，占一半
    load_pool.weights = weights
    return out


def pick(options):
    r, acc = random.random(), 0.0
    for name, p in options:
        acc += p
        if r < acc:
            return name
    return options[-1][0]


def league_pick(out_dir: Path) -> Path | None:
    """从输出目录的历史检查点里挑一个当对手：一半取最近 LEAGUE_RECENT 个，一半在全部历史里均匀取。"""
    cks = sorted((p for p in out_dir.glob("update-*.pt") if p.stem.split("-")[1].isdigit()),
                 key=lambda p: int(p.stem.split("-")[1]))
    if not cks:
        return None
    return random.choice(cks[-LEAGUE_RECENT:] if random.random() < 0.5 else cks)


_LEAGUE: dict = {}       # 子进程里的对手模型缓存：路径 → 已加载模型（最多 6 个）


def league_model(path: Path, device: str):
    key = str(path)
    if key not in _LEAGUE:
        if len(_LEAGUE) >= 6:
            _LEAGUE.pop(next(iter(_LEAGUE)))
        _LEAGUE[key] = load_policy_v4(key, device=device)
    return _LEAGUE[key]


def make_opponent(kind: str, side: int, pool: list[DeckSpec], snapshot, device: str, out_dir: Path | None = None):
    """返回 (对手, 卡组, 对手说明)。"""
    if kind == "rules":
        from crai.bot.rules import Hog26Bot
        return Hog26Bot(), HOG26_SPEC, "rules"
    if kind == "everlight":
        deck = random.choices(pool, weights=load_pool.weights)[0]
        return (FLAgent(side, {side: forms_of(deck)}, model=load_model_on("everlight", device), device=device),
                deck, "everlight")
    deck = random.choices(pool, weights=load_pool.weights)[0] if random.random() < 0.5 else HOG26_SPEC
    model, tag = snapshot, "init"
    if kind == "league" and out_dir is not None:
        ck = league_pick(out_dir)
        if ck is not None:
            model, tag = league_model(ck, device), ck.stem
    return FLAgent(side, {side: forms_of(deck)}, model=model, sample=OPP_SAMPLE, device=device), deck, tag


def reconnect(env: NativeEnv) -> NativeEnv:
    """引擎断开后：关掉旧连接，确认模拟器和游戏在运行（必要时由 headless_resim.ensure_game 重启本实例的游戏），再重连。"""
    from headless_resim import ensure_game
    try:
        env.close()
    except Exception:  # noqa: BLE001
        pass
    time.sleep(5)
    ensure_game()
    return NativeEnv(wait=300)


_LOADED = {}


def load_model_on(name: str, device: str):
    if name not in _LOADED:
        from crai.env.fl_agent import CHECKPOINTS
        _LOADED[name] = load_policy_v4(str(CHECKPOINTS[name]), device=device)
    return _LOADED[name]


# ---------------------------------------------------------------------------
# 并行采集：W 个常驻子进程，各自在 GPU 上加载模型、各连一个引擎连接（代理库多客户端，各自一局）
# ---------------------------------------------------------------------------
_W: dict = {}
TMP = ROOT / "data/ppo/tmp_episodes"


def _worker_init(init: str, device: str):
    os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
    random.seed(os.getpid() * 1000003 + int(time.time() * 1000))
    torch.set_num_threads(4)
    _W.update(device=device, init=init, version=None, learner=None, snapshot=load_policy_v4(init, device=device),
              pool=load_pool(), rcfg=RewardConfig())


def _worker_collect(task):
    """一个采集进程：同时跑 n_parallel 局（crai/env/vec_match.run_many，GPU 批量推理），凑满 n_games 局。
    task = (version, 学习方检查点路径, 更新编号, n_games, n_parallel)。返回 [(轨迹临时文件 | None, 本局摘要)]。"""
    from crai.env.match import MatchState
    from crai.env.vec_match import run_many
    version, ckpt, update_step, n_games, n_parallel, out_dir = task
    out_dir = Path(out_dir)
    W = _W
    if W["version"] != version:                       # 学习方权重更新了：重新加载
        W["learner"] = None
        if W["device"] == "cuda":
            torch.cuda.empty_cache()
        W["learner"] = load_policy_v4(ckpt, device=W["device"])
        W["version"] = version
    device, pool = W["device"], W["pool"]
    out = []
    started = [0]

    def make(env):
        ok = sum(p is not None for p, _ in out)
        in_flight = started[0] - len(out)
        if ok + in_flight >= n_games:                 # 已完成 + 在打的局够凑满了
            return None
        if started[0] >= n_games + 3 * n_parallel:    # 引擎反复崩溃：不再无限重开
            return None
        started[0] += 1
        side = random.randint(0, 1)
        kind = pick(OPPONENTS)
        opp, opp_deck, opp_tag = make_opponent(kind, 1 - side, pool, W["snapshot"], device, out_dir)
        learner = PPOLearner(side, {side: forms_of(HOG26_SPEC)}, W["learner"], device)
        agents = {side: learner, 1 - side: opp}
        decks = {side: HOG26_SPEC, 1 - side: opp_deck}
        seed = random.randint(1, 2**31 - 1)
        ms = MatchState(agents[0], agents[1], decks[0], decks[1], seed, env)
        ms.info = {"side": side, "kind": kind, "opp_tag": opp_tag, "opp_deck": opp_deck.deck, "seed": seed,
                   "learner": learner}
        return ms

    def done(ms, r):
        i = ms.info
        if r is None:                                 # 引擎崩溃，这局作废
            out.append((None, {"update": update_step, "crash": True, "opp": i["kind"], "seed": i["seed"]}))
            return
        side, learner = i["side"], i["learner"]
        steps = learner.session.steps if learner.session else []
        rewards, rinfo = episode_rewards(steps, r.crowns[side], r.crowns[1 - side], W["rcfg"])
        meta = {"update": update_step, "opp": i["kind"], "opp_tag": i["opp_tag"], "opp_deck": i["opp_deck"], "side": side, "seed": i["seed"],
                "crowns": [r.crowns[side], r.crowns[1 - side]], "ticks": r.ticks, "steps": len(steps),
                "plays": r.plays[side], "opp_plays": r.plays[1 - side], "secs": r.seconds,
                "return": round(sum(rewards), 4), **rinfo}
        TMP.mkdir(parents=True, exist_ok=True)
        path = TMP / f"{os.getpid()}_{time.time_ns()}.pt"
        torch.save({"steps": pack_steps(steps), "rewards": rewards}, path)   # 打包：主进程读回快 5 倍
        out.append((str(path), meta))

    t0 = time.time()
    st = run_many(make, n_parallel, done, log=lambda s: print(f"{timestamp()} [pid {os.getpid()}] {s}", flush=True))
    print(f"{timestamp()} [pid {os.getpid()} 实例 {os.environ.get('CRAI_INSTANCE')}] 采集 {len(out)} 局 {time.time() - t0:.0f}s："
          f"轮 {st['rounds']} 整理 {st['pre_s']:.0f}s 推理 {st['infer_s']:.0f}s 引擎 {st['step_s']:.0f}s", flush=True)
    return out


def main(n_updates="1", per_update="4", out_dir="models/ppo/run1", init=str(INIT)):
    import multiprocessing as mp
    sys.stdout.reconfigure(line_buffering=True)
    out = ROOT / out_dir
    out.mkdir(parents=True, exist_ok=True)
    device = "cuda" if torch.cuda.is_available() else "cpu"
    # 采集：n_workers 个进程，每个同时跑 n_parallel 局并批量推理（实测 2 × 10 最好：再多 GPU 就饱和了）
    n_workers = int(os.environ.get("CRAI_PPO_WORKERS", "2"))
    n_parallel = int(os.environ.get("CRAI_PPO_PARALLEL", "10"))
    start = out / "latest.pt" if (out / "latest.pt").exists() else Path(init)
    learner_policy = load_policy_v4(str(start), device=device)
    model = learner_policy.model
    optimizer = torch.optim.AdamW(model.parameters(), lr=float(os.environ.get("CRAI_PPO_LR", "1e-5")))
    if start.name == "latest.pt":
        payload = torch.load(start, map_location="cpu", weights_only=False)
        if payload.get("optimizer_state_dict"):
            optimizer.load_state_dict(payload["optimizer_state_dict"])
    update_step = learner_policy.update_step
    # 保留起点检查点自带的 extra（Everlight 还有 allowed_delay_offsets_ms 等），缺的用 EXTRA 补
    extra = dict(EXTRA)
    try:
        extra.update(torch.load(start, map_location="cpu", weights_only=False).get("extra") or {})
    except Exception as e:  # noqa: BLE001
        print(f"读起点 extra 失败：{e}")
    from crai.rl.expert_data import ExpertSampler
    sampler = ExpertSampler(learner_policy, n=int(os.environ.get("CRAI_BC_GAMES", "8")), seed=update_step)
    log = (out / "episodes.jsonl").open("a", encoding="utf-8")
    ulog = (out / "updates.jsonl").open("a", encoding="utf-8")
    # 两个实例都采集（2026-11-22）：实例 1（CRAI_INSTANCE，默认）常驻 n_workers 个进程；
    # 实例 0（天梯号）再加 CRAI_PPO_WORKERS0 个进程，只在没做天梯验证时用。每个进程 n_parallel 局，每局 per_update/每进程份额。
    # 天梯验证也由这里发起：每 CRAI_LADDER_EVERY 次更新，存完检查点（此时实例 0 上没有在打的离线局）就在实例 0 上
    # 起 ladder_validate.py once 子进程；它跑完之前实例 0 不参与采集。
    n_workers0 = int(os.environ.get("CRAI_PPO_WORKERS0", "1"))
    ladder_every = int(os.environ.get("CRAI_LADDER_EVERY", "10"))
    ladder_games = os.environ.get("CRAI_LADDER_GAMES", "5")
    main_inst = os.environ.get("CRAI_INSTANCE", "1")
    print(f"{timestamp()} 起点 {start}（update {update_step}），设备 {device}，实例 {main_inst} × {n_workers} 进程"
          f" + 实例 0 × {n_workers0} 进程，每进程 {n_parallel} 局并行，每次 {per_update} 局（实例 0 空闲时按比例加局）")
    ckpt = str(start)
    ctx = mp.get_context("spawn")
    workers = ctx.Pool(n_workers, initializer=_worker_init, initargs=(str(init), device))
    workers0 = None
    if n_workers0 > 0 and main_inst != "0":
        os.environ["CRAI_INSTANCE"] = "0"                 # spawn 子进程继承环境变量：这个池连实例 0
        try:
            workers0 = ctx.Pool(n_workers0, initializer=_worker_init, initargs=(str(init), device))
        finally:
            os.environ["CRAI_INSTANCE"] = main_inst
    ladder_proc = None
    # 验证节奏沿用输出目录里最早的检查点（run2 是 627 → 637、647…）
    _cks = sorted(int(p.stem.split("-")[1]) for p in out.glob("update-*.pt") if p.stem.split("-")[1].isdigit())
    ladder_first = _cks[0] if _cks else update_step
    from concurrent.futures import ThreadPoolExecutor
    prefetch = ThreadPoolExecutor(1)
    def shares(total, n):
        return [total // n + (1 if w < total % n else 0) for w in range(n)]

    def ladder_due(step):
        """step 这个检查点存完后要不要做天梯验证。"""
        if workers0 is None or ladder_every <= 0 or (step - ladder_first) % ladder_every:
            return False
        le = out / "ladder_eval.jsonl"
        if le.exists():
            return step not in {json.loads(x)["update"] for x in le.read_text(encoding="utf-8").splitlines() if x.strip()}
        return True

    def launch(version, ck, allow0):
        """异步开一轮采集。allow0=False：实例 0 不参与（在做天梯验证，或这轮采完就要做）。"""
        nonlocal ladder_proc
        if ladder_proc is not None and ladder_proc.poll() is not None:
            print(f"{timestamp()} 天梯验证结束（退出码 {ladder_proc.returncode}），实例 0 回到采集")
            ladder_proc = None
        use0 = workers0 is not None and ladder_proc is None and allow0
        per_main = int(per_update)
        per0 = round(per_main * n_workers0 / n_workers) if use0 else 0
        tasks = [(version, ck, version, n, min(n_parallel, n), str(out)) for n in shares(per_main, n_workers) if n]
        tasks0 = [(version, ck, version, n, min(n_parallel, n), str(out)) for n in shares(per0, n_workers0) if n]
        return workers.map_async(_worker_collect, tasks), (workers0.map_async(_worker_collect, tasks0) if tasks0 else None)

    def gather(pending):
        a, a0 = pending
        res = [x for chunk in a.get() for x in chunk]
        if a0 is not None:
            try:
                res += [x for chunk in a0.get(timeout=1800) for x in chunk]
            except Exception as e:  # noqa: BLE001 —— 实例 0 出问题不影响实例 1 这一轮
                print(f"{timestamp()} 实例 0 采集失败：{type(e).__name__}: {e}")
        return res

    # 采集和更新重叠（2026-11-22）：拿到这一轮的局后，立刻用当前检查点开下一轮采集，再做 PPO/BC 更新。
    # 下一轮用的模型落后一个版本；每次更新 approx_kl 只有约 0.0004，PPO 又按行为策略的概率做裁剪，影响可以忽略，
    # 但模拟器不再在更新的 5 分钟里空等。
    pending = launch(update_step, ckpt, True)
    try:
        for u in range(int(n_updates)):
            episodes = []
            t_up = time.time()
            # 人类回放序列（行为克隆用）构建慢（约 4 分钟），放到后台线程里和采集同时做
            bc_future = prefetch.submit(sampler.sample, update_step, 0, 1) if BC_COEF > 0 else None
            results = gather(pending)
            t_got = time.time()
            pending = None
            if u + 1 < int(n_updates):
                # 这轮更新完存 update_step+1；它要做天梯验证时，验证会在下一轮采集进行中开始，所以实例 0 这轮先不派
                pending = launch(update_step, ckpt, not ladder_due(update_step + 1))
            per_update_now = len(results)
            for g, (path, meta) in enumerate(results, 1):
                if path is None:
                    log.write(json.dumps(meta, ensure_ascii=False) + "\n"); log.flush()
                    print(f"{timestamp()}  局 {g} 作废（引擎崩溃）：{meta}")
                    continue
                data = torch.load(path, map_location="cpu", weights_only=False)
                if isinstance(data["steps"], dict):
                    data["steps"] = unpack_steps(data["steps"])
                Path(path).unlink(missing_ok=True)
                log.write(json.dumps(meta, ensure_ascii=False) + "\n"); log.flush()
                print(f"{timestamp()}  局 {g}/{per_update_now} vs {meta.get('opp_tag', meta['opp']):12s} 皇冠 {meta['crowns'][0]}:{meta['crowns'][1]} "
                      f"出牌 {meta['plays']}/{meta['opp_plays']}（对手） 步 {meta['steps']} 回报 {meta['return']:+.3f} "
                      f"溢出 {meta['wasted_elixir']} {meta['secs']}s")
                episodes.append(Episode(data["steps"], data["rewards"], meta))
            if not episodes:
                raise RuntimeError("这一轮没有采到任何一局")
            t_rl = time.time()
            rollout = build_rollout(episodes, model, policy_state_id=f"crai-ppo-{update_step}")
            metrics = ppo_update(model, optimizer, rollout)
            del rollout
            phase = {"wait": round(t_got - t_up), "load": round(t_rl - t_got), "ppo": round(time.time() - t_rl)}
            # 人类回放行为克隆（FirstLight 野猪专精配方）。强度可在训练中途调：<输出目录>/bc.json
            # {"coef": 系数, "steps": 每次更新做几步 BC}；没有该文件用 CRAI_BC_COEF / CRAI_BC_STEPS。
            # 梯度会被裁剪到 0.5，coef 调大作用有限，真正加大模仿力度靠 steps（同一批人类局重复学几步）。
            bc_cfg = {"coef": BC_COEF, "steps": int(os.environ.get("CRAI_BC_STEPS", "1"))}
            try:
                bc_cfg.update(json.loads((out / "bc.json").read_text(encoding="utf-8")))
            except (OSError, ValueError):
                pass
            bc = {}
            if bc_cfg["coef"] > 0 and bc_cfg["steps"] > 0:
                t_bc = time.time()
                reuse = _ReuseSampler(sampler)
                if bc_future is not None:                  # 采集期间后台已经构建好的序列
                    try:
                        reuse.items = bc_future.result()
                    except Exception as e:  # noqa: BLE001
                        print(f"{timestamp()} 预构建人类序列失败，现场再建：{type(e).__name__}: {e}")
                phase["bc_wait"] = round(time.time() - t_bc)
                try:
                    for _ in range(int(bc_cfg["steps"])):
                        bc = distributed_expert_bc_update_v4(
                            model, optimizer, reuse, update_step=update_step,
                            sequence_chunk_steps=int(bc_cfg.get("chunk", 48)),   # 块越大前后向次数越少、越快（bc.json 可调）
                            coefficient=float(bc_cfg["coef"]),
                            loss_weights=ILLossWeightsV4(value=0.0, delay=0.0, continue_second_weight=32.0)) or {}
                except Exception as e:  # noqa: BLE001 —— BC 出错不能丢掉 PPO 更新
                    bc = {"error": f"{type(e).__name__}: {e}"}
                for mod in model.modules():
                    if isinstance(mod, torch.nn.RNNBase):
                        mod.flatten_parameters()
                model.eval()
                bc["steps"] = int(bc_cfg["steps"])
                bc = {k: (round(v, 5) if isinstance(v, float) else v) for k, v in dict(bc).items()
                      if isinstance(v, (int, float, str))}
                bc["seconds"] = round(time.time() - t_bc, 1)
                phase["bc"] = round(time.time() - t_bc) - phase["bc_wait"]
            update_step += 1
            for name in ("latest.pt", f"update-{update_step:05d}.pt"):
                save_actor_critic_checkpoint(out / name, model, optimizer=optimizer, scheduler=None, grad_scaler=None,
                                             update_step=update_step, training_stage="crai-ppo",
                                             gamma_per_decision=0.9997, gae_lambda=0.98, extra=dict(extra))
            ckpt = str(out / f"update-{update_step:05d}.pt")      # 子进程下一轮加载这个
            wins = sum(e.meta["crowns"][0] > e.meta["crowns"][1] for e in episodes)
            passive = sum(e.meta["opp_plays"] <= 3 for e in episodes)
            last = metrics[-1] if metrics else {}
            rec = {"update": update_step, "games": len(episodes), "wins": wins, "passive_opp_games": passive,
                   "collect_s": round(t_rl - t_up, 1), "update_s": round(time.time() - t_rl, 1),
                   "metrics": {k: (round(v, 5) if isinstance(v, float) else v) for k, v in last.items()}, "bc": bc,
                   "phase_s": phase}
            ulog.write(json.dumps(rec, ensure_ascii=False) + "\n"); ulog.flush()
            print(f"{timestamp()} 更新 {update_step}：胜 {wins}/{len(episodes)}，对手消极局 {passive}，"
                  f"采集 {rec['collect_s']}s，更新 {rec['update_s']}s，各环节 {phase}，{rec['metrics']}，"
                  f"BC loss {bc.get('bc_loss')} 野猪概率 {bc.get('bc_expert_hog_joint_first_micro_prob')} {bc.get('error', '')}")
            if device == "cuda":
                torch.cuda.empty_cache()
            # 天梯验证：到点就在实例 0 上起验证子进程（这时实例 0 上没有离线局在打）
            if (workers0 is not None and ladder_every > 0 and ladder_proc is None
                    and (update_step - ladder_first) % ladder_every == 0):
                done_u = set()
                le = out / "ladder_eval.jsonl"
                if le.exists():
                    done_u = {json.loads(x)["update"] for x in le.read_text(encoding="utf-8").splitlines() if x.strip()}
                if update_step not in done_u:
                    import subprocess
                    env0 = dict(os.environ, CRAI_INSTANCE="0", PYTHONIOENCODING="utf-8")
                    lo = open(out / "ladder.out", "a", encoding="utf-8")
                    ladder_proc = subprocess.Popen(
                        [sys.executable, "-u", str(ROOT / "scripts/ladder_validate.py"), "once", str(out.relative_to(ROOT)),
                         ckpt, str(update_step), ladder_games], cwd=str(ROOT), env=env0, stdout=lo, stderr=subprocess.STDOUT)
                    print(f"{timestamp()} 开始天梯验证 update {update_step}（实例 0 暂停采集）")
    finally:
        workers.terminate()
        if workers0 is not None:
            workers0.terminate()
        prefetch.shutdown(wait=False, cancel_futures=True)
        log.close(); ulog.close()


if __name__ == "__main__":
    main(*sys.argv[1:])
