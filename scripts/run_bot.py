"""在训练营里连续自动对战 N 局（规则 bot）。
每局：开训练营 -> 读内存决策下牌 -> 识别结算界面 -> 记录结果到 data/bot_runs/<时间>.jsonl
用法: python -u scripts/run_bot.py [局数=10]
"""
import json
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.bot.rules import NAME, Hog26Bot, to_own_frame  # noqa: E402
from crai.client.control import Screen, deploy, deselect  # noqa: E402
from crai.reader.stream import LiveReader  # noqa: E402
from ui import shot  # noqa: E402
import start_training  # noqa: E402


def on_result_screen() -> bool:
    from ui import screen_state
    return screen_state() == "result"


def tower_summary(frame):
    v = to_own_frame(frame)
    return {"own_towers": len(v.own_towers), "enemy_towers": len(v.enemy_towers),
            "own_tower_hp": sum(t.hp for t in v.own_towers if t.hp > 0),
            "enemy_tower_hp": sum(t.hp for t in v.enemy_towers if t.hp > 0)}


BOT = "rules"


def play_one(scr, log) -> dict:
    if BOT == "il":
        from crai.bot.il_bot import ILBot
        bot = ILBot()
    elif BOT == "v2":
        from crai.bot.v2_bot import V2Bot
        bot = V2Bot()
    else:
        bot = Hog26Bot()
    live = LiveReader(period_ms=30, rescan=3)
    t0, n, last_check, plays, last = time.time(), -1, time.time(), 0, None
    try:
        while time.time() - t0 < 420:
            n, f = live.latest(newer_than=n, timeout=10)
            if f.battle and f.players:
                if last is None:  # 记录己方 side 和国王塔在内存中的位置（验证坐标翻转）
                    me = f.players[f.own_index()]
                    kings = {e.side: e.y for e in f.entities if e.kind in (12, 13) and e.card == -1 and e.x == 9000}
                    info = {"own_side": me.side, "own_king_y": kings.get(me.side), "enemy_king_y": kings.get(1 - me.side),
                            "own_deck": me.deck}
                    log.write(json.dumps({"type": "start", **info}) + "\n")
                    print(f"  开局: {info}")
                last = f
                act = bot.decide(f)
                if bot.need_deselect:
                    deselect(); bot.need_deselect = False
                if act:
                    px, py = deploy(scr, act.slot, act.x, act.y)
                    plays += 1
                    rec = {"t": round(time.time() - t0, 2), "card": act.card, "x": act.x, "y": act.y,
                           "px": px, "py": py, "reason": act.reason}
                    log.write(json.dumps({"type": "play", **rec}, ensure_ascii=False) + "\n")
                    print(f"  [{rec['t']:6.1f}s] {NAME.get(act.card, act.card)} -> ({act.x},{act.y}) {act.reason}")
            if time.time() - last_check > 3:
                last_check = time.time()
                if time.time() - t0 > 30 and on_result_screen():
                    break
    finally:
        live.close()
    res = {"duration": round(time.time() - t0, 1), "plays": plays, "failed_plays": bot.failed_plays}
    if last is not None:
        res.update(tower_summary(last))
    # 结果判定：对方塔数比己方少 -> 胜（训练营 3 塔起始；被拆的塔不再出现在实体中）
    if "own_towers" in res:
        res["result"] = "win" if res["enemy_towers"] < res["own_towers"] else (
            "lose" if res["enemy_towers"] > res["own_towers"] else "draw")
    return res


def main(n_games=10, mode="training"):
    sys.stdout.reconfigure(line_buffering=True)
    out_dir = ROOT / "data" / "bot_runs"; out_dir.mkdir(parents=True, exist_ok=True)
    path = out_dir / f"{datetime.now():%Y%m%d_%H%M%S}.jsonl"
    scr = Screen()
    summary = []
    with path.open("w", encoding="utf-8") as log:
        for g in range(n_games):
            if start_training.main(mode) != 0:
                print("开局失败，重试一次"); time.sleep(5)
                if start_training.main(mode) != 0:
                    break
            print(f"== 第 {g + 1} 局开始")
            res = play_one(scr, log)
            res["game"] = g + 1
            log.write(json.dumps({"type": "result", **res}, ensure_ascii=False) + "\n"); log.flush()
            summary.append(res)
            print(f"== 第 {g + 1} 局结束: {res}")
            time.sleep(2)
    wins = sum(r.get("result") == "win" for r in summary)
    print(f"共 {len(summary)} 局，胜 {wins}；日志 {path}")


if __name__ == "__main__":
    if len(sys.argv) > 3:
        BOT = sys.argv[3]
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 10, sys.argv[2] if len(sys.argv) > 2 else "training")
