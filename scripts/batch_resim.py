"""批量重演：对 replay/ 中出牌数不超过底子容量的回放逐个注入 + 录制，汇总结局核对。
已有 data/resim/<名>.jsonl 的跳过。结果追加到 data/resim/_summary.jsonl
用法: python -u scripts/batch_resim.py [容量=57]
"""
import glob
import json
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.data import cards  # noqa: E402
from crai.data.replay import load_replay  # noqa: E402
import inject_replay  # noqa: E402
from ui import go_home  # noqa: E402


def main(cap=57):
    sys.stdout.reconfigure(line_buffering=True)
    out = ROOT / "data" / "resim"; out.mkdir(parents=True, exist_ok=True)
    todo = []
    for p in sorted(glob.glob(str(ROOT / "replay" / "replay*.json"))):
        r = load_replay(p)
        n = sum(1 for x in r.plays if not x.is_ability and cards.card_id(x.card) != -1)
        if n <= cap and not (out / (Path(p).stem + ".jsonl")).exists():
            todo.append(p)
    print(f"待重演 {len(todo)} 局")
    for i, p in enumerate(todo):
        print(f"== [{i + 1}/{len(todo)}] {Path(p).name}")
        try:
            res = inject_replay.main(p)
        except SystemExit as e:
            res = {"file": Path(p).name, "error": str(e)}
        except Exception as e:  # 单局失败不影响后续
            res = {"file": Path(p).name, "error": repr(e)}
        with (out / "_summary.jsonl").open("a", encoding="utf-8") as fh:
            fh.write(json.dumps(res, ensure_ascii=False) + "\n")
        print("  ", res)
        time.sleep(3)
        go_home()


if __name__ == "__main__":
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 57)
