"""PPO 检查点的固定评测：确定性出牌（argmax），固定种子和固定对手卡组，可以横向比较不同检查点。

对手（每种 N 局，交替换边）：
  - everlight：FirstLight 通用模型，用卡组池里按固定种子抽出的天梯卡组；
  - init：PPO 的起点模型（迁移后的野猪专精 2 号），同样的卡组池卡组；
  - rules：规则 bot，野猪镜像。
被测模型始终用野猪 2.6（用户形态）。每局记录对手出牌数，对手出牌 ≤ 3 的"消极局"单独列出。

默认在实例 0 上跑（实例 1 留给训练和重演）：
  $env:CRAI_FL_ROOT="yesdelay"; $env:CRAI_INSTANCE="0"
  .venv\\Scripts\\python.exe -u scripts/ppo_eval.py <检查点 .pt> [每种对手局数 10] [对手 everlight,init,rules]
结果写到 models/ppo/eval/<检查点名>.json。
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

from crai.env.fl_agent import FLAgent  # noqa: E402
from crai.env.match import HOG26_SPEC, play_match  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402
from ppo_train import INIT, forms_of, load_model_on, load_pool, reconnect  # noqa: E402
from native_runner.training.v4.policy_session import load_policy_v4  # noqa: E402

SEED = 20261121


def eval_decks(n: int):
    """评测用的对手卡组：固定种子从卡组池抽取。抽取规则固定为最初的版本（只要求卡牌在引擎卡牌表里），
    训练用的卡组池以后再怎么过滤，评测卡组都不变，各检查点的结果才可比。"""
    from crai.data import cards
    from crai.env.match import DeckSpec
    from ppo_train import POOL
    raw = json.loads(POOL.read_text(encoding="utf-8"))
    pool = [d for d in raw if all(s in cards.CARDS for s in d["deck"])]
    rng = random.Random(SEED)
    out = []
    for _ in range(n):
        d = rng.choices(pool, weights=[d["count"] for d in pool])[0]
        out.append(DeckSpec(d["deck"], d["forms"], tower=d["tower"]))
    return out


def main(ckpt: str, n: str = "10", opponents: str = "everlight,init,rules"):
    sys.stdout.reconfigure(line_buffering=True)
    device = "cuda" if torch.cuda.is_available() else "cpu"
    cand = load_policy_v4(ckpt, device=device)
    init = load_policy_v4(str(INIT), device=device)
    decks = eval_decks(int(n))                                       # 每次评测都一样
    env = NativeEnv(wait=300)
    rows = []
    t0 = time.time()
    try:
        for kind in opponents.split(","):
            for i in range(int(n)):
                side = i % 2
                opp_deck = HOG26_SPEC if kind == "rules" else decks[i]
                me = FLAgent(side, {side: forms_of(HOG26_SPEC)}, model=cand, device=device)
                if kind == "rules":
                    from crai.bot.rules import Hog26Bot
                    opp = Hog26Bot()
                else:
                    pol = load_model_on("everlight", device) if kind == "everlight" else init
                    opp = FLAgent(1 - side, {1 - side: forms_of(opp_deck)}, model=pol, device=device)
                agents = {side: me, 1 - side: opp}
                dk = {side: HOG26_SPEC, 1 - side: opp_deck}
                for attempt in range(3):
                    try:
                        r = play_match(agents[0], agents[1], dk[0], dk[1], seed=SEED + i, env=env)
                        break
                    except (ConnectionError, OSError) as e:
                        print(f"  引擎断开（{e}），重连后重打这一局")
                        env = reconnect(env)
                        me = FLAgent(side, {side: forms_of(HOG26_SPEC)}, model=cand, device=device)
                        if kind != "rules":
                            opp = FLAgent(1 - side, {1 - side: forms_of(opp_deck)}, model=pol, device=device)
                        else:
                            opp = Hog26Bot()
                        agents = {side: me, 1 - side: opp}
                else:
                    continue
                c = [r.crowns[side], r.crowns[1 - side]]
                row = {"opp": kind, "i": i, "side": side, "opp_deck": opp_deck.deck, "crowns": c,
                       "result": (c[0] > c[1]) - (c[0] < c[1]), "plays": r.plays[side],
                       "opp_plays": r.plays[1 - side], "ticks": r.ticks}
                rows.append(row)
                print(f"  {kind:9s} 第 {i + 1:2d} 局 {c[0]}:{c[1]}  出牌 {row['plays']}/{row['opp_plays']}（对手）"
                      f"  tick {r.ticks}  {r.seconds}s")
    finally:
        env.close()
    summary = {"checkpoint": ckpt, "games_per_opp": int(n), "minutes": round((time.time() - t0) / 60, 1), "by_opp": {}}
    for kind in opponents.split(","):
        rs = [r for r in rows if r["opp"] == kind]
        if not rs:
            continue
        summary["by_opp"][kind] = {
            "games": len(rs), "win": sum(r["result"] > 0 for r in rs), "draw": sum(r["result"] == 0 for r in rs),
            "loss": sum(r["result"] < 0 for r in rs),
            "crowns": [sum(r["crowns"][0] for r in rs), sum(r["crowns"][1] for r in rs)],
            "passive_opp_games": sum(r["opp_plays"] <= 3 for r in rs)}
    out = ROOT / "models/ppo/eval" / (Path(ckpt).parent.name + "_" + Path(ckpt).stem + ".json")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps({"summary": summary, "rows": rows}, ensure_ascii=False, indent=1), encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False))


if __name__ == "__main__":
    main(*sys.argv[1:])
