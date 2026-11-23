"""盯住 PPO 训练目录，每隔 every 次更新自动评测一次检查点（ppo_eval，默认实例 0），并打印汇总表。

  $env:CRAI_FL_ROOT="yesdelay"; $env:CRAI_INSTANCE="0"
  .venv\\Scripts\\python.exe -u scripts/ppo_eval_watch.py [训练目录 models/ppo/run1] [every 10] [每种对手局数 10]
只看汇总：.venv\\Scripts\\python.exe scripts/ppo_eval_watch.py table [训练目录]
"""
import json
import os
import re
import sys
import time
from pathlib import Path

os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
EVAL = ROOT / "models/ppo/eval"


def result_path(ckpt: Path) -> Path:
    return EVAL / f"{ckpt.parent.name}_{ckpt.stem}.json"


def table(run: Path) -> None:
    rows = []
    base = EVAL / "ppo_hog2_yesdelay_init.json"
    files = ([base] if base.exists() else []) + sorted(EVAL.glob(f"{run.name}_update-*.json"))
    for f in files:
        s = json.loads(f.read_text(encoding="utf-8"))["summary"]
        cells = []
        for kind in ("everlight", "init", "rules"):
            b = s["by_opp"].get(kind)
            cells.append(f"{b['win']}-{b['draw']}-{b['loss']} 冠{b['crowns'][0]}:{b['crowns'][1]}"
                         + (f" 消极{b['passive_opp_games']}" if b["passive_opp_games"] else "") if b else "-")
        name = "起点" if f == base else f.stem.split("_", 1)[1]
        rows.append(f"{name:14s} | " + " | ".join(f"{c:18s}" for c in cells))
    print(f"{'检查点':14s} | {'vs Everlight 胜-平-负':18s} | {'vs 起点模型':18s} | {'vs 规则 bot':18s}")
    print("\n".join(rows))


def main(run_dir="models/ppo/run1", every="10", n="10"):
    sys.stdout.reconfigure(line_buffering=True)
    import ppo_eval
    run = ROOT / run_dir
    while True:
        todo = []
        for p in sorted(run.glob("update-*.pt")):
            k = int(re.search(r"update-(\d+)", p.stem).group(1))
            if (k - 616) % int(every) == 0 and not result_path(p).exists():
                todo.append(p)
        for p in todo:
            print(f"{time.strftime('%H:%M:%S')} 评测 {p.name}")
            ppo_eval.main(str(p), n)
            table(run)
        time.sleep(300)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "table":
        table(ROOT / (sys.argv[2] if len(sys.argv) > 2 else "models/ppo/run1"))
    else:
        main(*sys.argv[1:])
