"""天梯实战学习（PPO）：在天梯上用采样策略打真人，记录每个决策步（观测、动作、log π、价值），
每打满 N 局用这些对局做一次 PPO 更新，然后用新权重继续打。

数据来源（用户 2026-11-22 定的方向：训练只从人类回放和天梯实战学习）：
  - PPO：只用天梯对局；
  - 每次 PPO 更新后一步行为克隆（FirstLight distributed_expert_bc_update_v4，系数 CRAI_BC_COEF=0.2）：
    随机 8 局 HF 人类野猪赢局（用户形态，新代理库重演，crai/rl/expert_data.py）。

  $env:CRAI_FL_ROOT="yesdelay"; $env:CRAI_INSTANCE="0"
  .venv\\Scripts\\python.exe -u scripts/ladder_learn.py [局数 20] [每次更新局数 8] [输出目录 models/ladder/run1] [起点检查点]

- 每局轨迹存到 <输出目录>/episodes/<时间>.pt（steps + rewards + meta），以后可重复使用；
- 断点续跑：输出目录有 latest.pt 就从它继续（含优化器），未用于更新的对局文件会被下一次更新使用；
- 奖励与离线 PPO 相同：胜负 ±1 + 塔血势能 0.05 + 圣水溢出惩罚（crai/rl/ppo_fl.py）。
"""
import json
import os
import sys
import time
from datetime import datetime
from pathlib import Path

os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))

import torch  # noqa: E402

import start_training  # noqa: E402
from crai.client.control import Screen  # noqa: E402
from crai.env.match import crowns_of  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402
from crai.rl.ppo_fl import Episode, PPOLearner, RewardConfig, build_rollout, episode_rewards, ppo_update, timestamp  # noqa: E402
from crai.rl.expert_data import ExpertSampler  # noqa: E402
from native_runner.training.v4.checkpoint import save_actor_critic_checkpoint  # noqa: E402
from native_runner.training.v4.imitation import ILLossWeightsV4  # noqa: E402
from native_runner.training.v4.ppo_expert_bc import distributed_expert_bc_update_v4  # noqa: E402

BC_COEF = float(os.environ.get("CRAI_BC_COEF", "0.2"))      # 0 关闭人类回放行为克隆
from native_runner.training.v4.policy_session import load_policy_v4  # noqa: E402
from ppo_train import EXTRA  # noqa: E402
from run_fl_live import play_one  # noqa: E402

DEFAULT_INIT = ROOT / "models/ppo/run1/update-00627.pt"


def main(n_games="20", per_update="8", out_dir="models/ladder/run1", init=str(DEFAULT_INIT)):
    sys.stdout.reconfigure(line_buffering=True)
    out = ROOT / out_dir
    (out / "episodes").mkdir(parents=True, exist_ok=True)
    device = "cuda" if torch.cuda.is_available() else "cpu"
    start = out / "latest.pt" if (out / "latest.pt").exists() else Path(init)
    policy = load_policy_v4(str(start), device=device)
    model = policy.model
    model.eval()
    optimizer = torch.optim.AdamW(model.parameters(), lr=float(os.environ.get("CRAI_PPO_LR", "1e-5")))
    if start.name == "latest.pt":
        payload = torch.load(start, map_location="cpu", weights_only=False)
        if payload.get("optimizer_state_dict"):
            optimizer.load_state_dict(payload["optimizer_state_dict"])
    update_step = policy.update_step
    state_path = out / "state.json"
    state = json.loads(state_path.read_text(encoding="utf-8")) if state_path.exists() else {"used": []}
    cfg = RewardConfig()
    sampler = ExpertSampler(policy, n=int(os.environ.get("CRAI_BC_GAMES", "8")), seed=update_step)
    scr = Screen()
    env = NativeEnv()
    glog = (out / "games.jsonl").open("a", encoding="utf-8")
    ulog = (out / "updates.jsonl").open("a", encoding="utf-8")
    live_log = (ROOT / "data/bot_runs" / f"ladder_learn_{datetime.now():%Y%m%d_%H%M%S}.jsonl").open("w", encoding="utf-8")
    print(f"{timestamp()} 起点 {start}（update {update_step}），设备 {device}，每 {per_update} 局更新一次")
    try:
        for g in range(int(n_games)):
            if start_training.main("ladder") != 0:
                print("开局失败，30 秒后重试")
                time.sleep(30)
                continue
            hold = {}
            res = play_one(scr, env, live_log, None,
                           make_agent=lambda own, forms: PPOLearner(own, forms, policy, device), hold=hold)
            agent, raw = hold.get("agent"), hold.get("raw")
            steps = agent.session.steps if agent is not None and agent.session is not None else []
            if not steps or raw is None or res.get("fallback"):
                print(f"{timestamp()} 第 {g + 1} 局没有可用轨迹（{res}），不用于训练")
                glog.write(json.dumps({"game": g + 1, "used": False, **res}, ensure_ascii=False) + "\n"); glog.flush()
                continue
            cr = crowns_of(raw)
            me = agent.owner
            rewards, info = episode_rewards(steps, cr[me], cr[1 - me], cfg)
            meta = {"time": datetime.now().isoformat(timespec="seconds"), "update": update_step, "crowns": [cr[me], cr[1 - me]],
                    "steps": len(steps), "return": round(sum(rewards), 3), "abilities": agent.abilities, **info, **res}
            path = out / "episodes" / f"{datetime.now():%Y%m%d_%H%M%S}.pt"
            torch.save({"steps": steps, "rewards": rewards, "meta": meta}, path)
            glog.write(json.dumps({"game": g + 1, "used": True, "file": path.name, **meta}, ensure_ascii=False) + "\n")
            glog.flush()
            print(f"{timestamp()} 第 {g + 1} 局 皇冠 {cr[me]}:{cr[1 - me]} 步 {len(steps)} 回报 {meta['return']:+.3f} "
                  f"技能 {agent.abilities} 溢出 {info['wasted_elixir']}")
            fresh = sorted(p for p in (out / "episodes").glob("*.pt") if p.name not in state["used"])
            if len(fresh) < int(per_update):
                continue
            eps = []
            for p in fresh:
                d = torch.load(p, map_location="cpu", weights_only=False)
                eps.append(Episode(d["steps"], d["rewards"], d["meta"]))
            t0 = time.time()
            rollout = build_rollout(eps, model, policy_state_id=f"crai-ladder-{update_step}")
            metrics = ppo_update(model, optimizer, rollout)
            del rollout
            # 人类回放行为克隆：每次 PPO 更新后一步（FirstLight 野猪专精配方：系数 0.2，8 局人类赢家野猪局）
            bc = {}
            if BC_COEF > 0:
                t_bc = time.time()
                try:
                    # 延迟头权重 0：我们的模型用 earliest 延迟策略（ppo_train.EXTRA），FirstLight 说明此时不训练嘈杂的延迟头
                    bc = distributed_expert_bc_update_v4(
                        model, optimizer, sampler, update_step=update_step, sequence_chunk_steps=48,
                        coefficient=BC_COEF,
                        loss_weights=ILLossWeightsV4(value=0.0, delay=0.0, continue_second_weight=32.0)) or {}
                except Exception as e:  # noqa: BLE001 —— BC 出错不能丢掉 PPO 更新
                    bc = {"error": f"{type(e).__name__}: {e}"}
                model.eval()
                bc = {k: (round(v, 5) if isinstance(v, float) else v) for k, v in dict(bc).items()
                      if isinstance(v, (int, float, str))}
                bc["seconds"] = round(time.time() - t_bc, 1)
                bc["games"] = [s.get("tag") for s in sampler.last_stats]
            update_step += 1
            for name in ("latest.pt", f"update-{update_step:05d}.pt"):
                save_actor_critic_checkpoint(out / name, model, optimizer=optimizer, scheduler=None, grad_scaler=None,
                                             update_step=update_step, training_stage="crai-ladder-ppo",
                                             gamma_per_decision=0.9997, gae_lambda=0.98, extra=dict(EXTRA))
            state["used"] += [p.name for p in fresh]
            state_path.write_text(json.dumps(state), encoding="utf-8")
            wins = sum(e.meta["crowns"][0] > e.meta["crowns"][1] for e in eps)
            last = metrics[-1] if metrics else {}
            rec = {"update": update_step, "games": len(eps), "wins": wins, "update_s": round(time.time() - t0, 1),
                   "metrics": {k: (round(v, 5) if isinstance(v, float) else v) for k, v in last.items()}, "bc": bc}
            ulog.write(json.dumps(rec, ensure_ascii=False) + "\n"); ulog.flush()
            print(f"{timestamp()} 更新 {update_step}：本批 {len(eps)} 局胜 {wins}，{rec['update_s']}s，"
                  f"kl {rec['metrics'].get('approx_kl')} ev {rec['metrics'].get('value_explained_variance')} "
                  f"BC {({k: bc[k] for k in bc if k != 'games'})}")
            if device == "cuda":
                torch.cuda.empty_cache()
    finally:
        env.close()
        glog.close(); ulog.close(); live_log.close()


if __name__ == "__main__":
    main(*sys.argv[1:])
