"""接手当前已开始的天梯对局，再自动打 2 局天梯（共 3 局）。"""
import json
import sys
import time
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import run_bot  # noqa: E402
import start_training  # noqa: E402
from crai.client.control import Screen  # noqa: E402

sys.stdout.reconfigure(line_buffering=True)
scr = Screen()
path = ROOT / "data" / "bot_runs" / f"ladder_{datetime.now():%Y%m%d_%H%M%S}.jsonl"
with path.open("w", encoding="utf-8") as log:
    for g in range(3):
        if g > 0:
            if start_training.main("ladder") != 0:
                print("开局失败"); break
        print(f"== 天梯第 {g + 1} 局")
        res = run_bot.play_one(scr, log)
        res["game"] = g + 1
        log.write(json.dumps({"type": "result", **res}, ensure_ascii=False) + "\n"); log.flush()
        print(f"== 第 {g + 1} 局结束: {res}")
        time.sleep(2)
print("日志:", path)
