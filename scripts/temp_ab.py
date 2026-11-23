"""实战出牌温度 A/B：同一个模型、同一组种子和对手卡组，只换"出不出牌 / 选牌选格子"的采样方式。

依据：Clash-Royale-Battle-Engine 的 docs/TRAINING.md §9–10：选牌和格子用低温（0.05≈argmax），
出牌时机保持 T=1 采样最好；时机温度偏离 1（0.7 / 1.3）明显变差。我们实战一直是三个头全 argmax。

模式：
  argmax  —— 全部 argmax（现行）
  gate    —— 时机（gate、continue）T=1 采样，选牌/格子/延迟 T=0.05
  full    —— 全部 T=1 采样
对手：Everlight + 卡组池卡组（与 ppo_eval 同一组评测卡组），交替换边。

  $env:CRAI_FL_ROOT="yesdelay"; $env:CRAI_INSTANCE="1"
  .venv\\Scripts\\python.exe -u scripts/temp_ab.py <检查点> [每种模式局数 40] [模式 argmax,gate,full] [并行 10]
结果写到 models/ppo/eval/temp_ab_<检查点名>.json。
"""
import json
import os
import sys
import time
from pathlib import Path

os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))

import torch  # noqa: E402

from crai.env.fl_agent import FLAgent  # noqa: E402
from crai.env.match import HOG26_SPEC, MatchState  # noqa: E402
from crai.env.vec_match import run_many  # noqa: E402
from ppo_eval import SEED, eval_decks  # noqa: E402
from ppo_train import forms_of, load_model_on  # noqa: E402
from native_runner.training.v4.policy_session import load_policy_v4  # noqa: E402

MODES = {"argmax": None, "gate": (1.0, 0.05, 1.0), "full": (1.0, 1.0, 1.0)}


def run_mode(ckpt: str, mode: str, n: int, par: int, device: str, decks) -> list[dict]:
    cand = load_policy_v4(ckpt, device=device)           # 每种模式单独一份模型（温度是模型级设置）
    temps = MODES[mode]
    if temps:
        m = cand.model
        m.set_ppo_rollout_sampling(True)
        m.set_ppo_gate_temperature(temps[0])
        m.set_ppo_action_temperature(temps[1])
        m.set_ppo_continue_temperature(temps[2])
    ever = load_model_on("everlight", device)
    todo = list(range(n))
    rows = []

    def make(env):
        if not todo:
            return None
        i = todo.pop(0)
        side = i % 2
        opp_deck = decks[i]
        me = FLAgent(side, {side: forms_of(HOG26_SPEC)}, model=cand, device=device, sample=temps is not None)
        opp = FLAgent(1 - side, {1 - side: forms_of(opp_deck)}, model=ever, device=device)
        agents = {side: me, 1 - side: opp}
        dk = {side: HOG26_SPEC, 1 - side: opp_deck}
        ms = MatchState(agents[0], agents[1], dk[0], dk[1], SEED + i, env)
        ms.info = {"i": i, "side": side}
        return ms

    def done(ms, r):
        i, side = ms.info["i"], ms.info["side"]
        if r is None:
            todo.append(i)                                # 引擎崩溃作废：重打
            return
        c = [r.crowns[side], r.crowns[1 - side]]
        row = {"mode": mode, "i": i, "side": side, "crowns": c, "result": (c[0] > c[1]) - (c[0] < c[1]),
               "plays": r.plays[side], "opp_plays": r.plays[1 - side], "ticks": r.ticks}
        rows.append(row)
        print(f"  {mode:6s} 第 {i + 1:2d} 局 {c[0]}:{c[1]}  出牌 {row['plays']}/{row['opp_plays']}  tick {r.ticks}",
              flush=True)

    run_many(make, par, done)
    return rows


def summarize(rows):
    return {"games": len(rows), "win": sum(r["result"] > 0 for r in rows),
            "draw": sum(r["result"] == 0 for r in rows), "loss": sum(r["result"] < 0 for r in rows),
            "crowns": [sum(r["crowns"][0] for r in rows), sum(r["crowns"][1] for r in rows)],
            "avg_plays": round(sum(r["plays"] for r in rows) / max(1, len(rows)), 1),
            "passive_opp_games": sum(r["opp_plays"] <= 3 for r in rows)}


def main(ckpt: str, n: str = "40", modes: str = "argmax,gate,full", par: str = "10"):
    sys.stdout.reconfigure(line_buffering=True)
    device = "cuda" if torch.cuda.is_available() else "cpu"
    decks = eval_decks(int(n))
    tag = os.environ.get("CRAI_AB_TAG", "")              # 区分不同观测/代理库版本的结果文件
    out = ROOT / "models/ppo/eval" / f"temp_ab_{Path(ckpt).parent.name}_{Path(ckpt).stem}{tag}.json"
    data = json.loads(out.read_text(encoding="utf-8")) if out.exists() else {"checkpoint": ckpt, "modes": {}}
    for mode in modes.split(","):
        t0 = time.time()
        rows = run_mode(ckpt, mode, int(n), int(par), device, decks)
        s = summarize(rows)
        s["minutes"] = round((time.time() - t0) / 60, 1)
        data["modes"][mode] = {"summary": s, "rows": rows}
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(data, ensure_ascii=False, indent=1), encoding="utf-8")
        print(mode, json.dumps(s, ensure_ascii=False), flush=True)
    for mode, v in data["modes"].items():
        print(f"{mode:6s} {json.dumps(v['summary'], ensure_ascii=False)}")


if __name__ == "__main__":
    main(*sys.argv[1:])
