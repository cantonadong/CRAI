"""打开对战日志第一局回放，开播后每 1.5s 扫描一次出牌指令，观察指令何时出现、数量与 tick 范围。"""
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.device import ADB, SERIAL, as_root  # noqa: E402
from ui import go_home, shot, tap  # noqa: E402


def sh(cmd):
    return subprocess.run([ADB, "-s", SERIAL, "shell", cmd], capture_output=True, text=True).stdout


def main(rounds=20, open_replay=True):
    sys.stdout.reconfigure(line_buffering=True)
    if open_replay:
        assert go_home()
        tap(983, 203); time.sleep(3); tap(647, 130); time.sleep(4); tap(900, 917)
    t0 = time.time()
    pid = sh("pidof nullsroyale.rel.free").strip()
    for k in range(rounds):
        lines = sh(as_root(f"/data/local/tmp/list_cmds {pid}")).splitlines()
        rows = [l.split() for l in lines if len(l.split()) == 7]
        ticks = sorted(int(r[1]) for r in rows)
        print(f"{time.time() - t0:5.1f}s 指令 {len(ticks)} 条 {(ticks[0], ticks[-1]) if ticks else ''}")
        if k == 3:
            shot("rp_load")
        time.sleep(1.5)


if __name__ == "__main__":
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 20, (sys.argv[2] != "0") if len(sys.argv) > 2 else True)
