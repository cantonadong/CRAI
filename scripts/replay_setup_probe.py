"""打开对战日志第 N 局回放，开播后立即抓取：roster、双方卡组容器/手牌/队列、全部出牌指令；
并尝试解码指令 +0x48 打包字段与 deck index 的关系。结果写入 re/replay_setup_<n>.txt
用法: python -u scripts/replay_setup_probe.py [第几局=1]
"""
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.device import ADB, SERIAL, as_root  # noqa: E402
from crai.reader.stream import snapshot  # noqa: E402
from ui import go_home, tap  # noqa: E402

WATCH_Y = {1: 917, 2: 1500}


def sh(cmd):
    return subprocess.run([ADB, "-s", SERIAL, "shell", cmd], capture_output=True, text=True).stdout


def dump_packed(pid, addr):
    out = sh(as_root(f"/data/local/tmp/dump_records {pid} {addr:x} 96 1 0"))
    return out.strip()


def main(n=1):
    sys.stdout.reconfigure(line_buffering=True)
    assert go_home()
    tap(983, 203); time.sleep(3); tap(647, 130); time.sleep(4); tap(900, WATCH_Y[n])
    time.sleep(2.5)
    pid = sh("pidof nullsroyale.rel.free").strip()
    cmds = sh(as_root(f"/data/local/tmp/list_cmds {pid}"))
    f = snapshot()
    decks = sh(as_root(f"/data/local/tmp/dump_decks {pid} {f.roster}")) if f.battle else "(未在对战)"
    rows = sorted((l.split() for l in cmds.splitlines() if len(l.split()) == 7), key=lambda r: int(r[1]))
    # 每条指令的 +0x48 打包字段
    packed = []
    for r in rows:
        a = int(r[0], 16)
        raw = sh(as_root(f"/data/local/tmp/dump_records {pid} {a + 0x40:x} 16 1 0"))
        packed.append(raw.split(":", 1)[1].strip() if ":" in raw else raw)
    out = ROOT / "re" / f"replay_setup_{n}.txt"
    with out.open("w", encoding="utf-8") as fh:
        fh.write(f"roster={f.roster}\n{decks}\n指令 {len(rows)} 条（addr ta tb player x y card | +0x40..+0x4f）\n")
        for r, p in zip(rows, packed):
            fh.write(" ".join(r) + " | " + p + "\n")
    print(out.read_text(encoding="utf-8")[:4000])


if __name__ == "__main__":
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 1)
