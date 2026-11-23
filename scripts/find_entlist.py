"""在对局中多次运行 entlist，找出"持有者地址稳定、内容始终覆盖当前全部实体"的实体列表容器。
同时让 bot 在后台打这局训练营，制造实体的增减。
用法: python -u scripts/find_entlist.py [采样次数=6] [间隔秒=8]
"""
import re
import subprocess
import sys
import time
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.device import ADB, SERIAL, as_root  # noqa: E402


def sample():
    pid = subprocess.run([ADB, "-s", SERIAL, "shell", "pidof nullsroyale.rel.free"], capture_output=True, text=True).stdout.strip()
    r = subprocess.run([ADB, "-s", SERIAL, "shell", as_root(f"/data/local/tmp/entlist {pid}")], capture_output=True, text=True)
    ne = re.search(r"entities=(\d+)", r.stderr)
    holders = {}
    for m in re.finditer(r"holder=(0x[0-9a-f]+) -> arr=(0x[0-9a-f]+) \(-(0x[0-9a-f]+)\) next=\[([-\d,]+)\]( \[libg\])?", r.stdout):
        holders[m[1]] = (m[2], int(m[3], 16), m[4], bool(m[5]))
    arrs = re.findall(r"arr=(0x[0-9a-f]+) len=(\d+) hit=(\d+)/(\d+)", r.stdout)
    return int(ne[1]) if ne else 0, holders, arrs


def main(n=6, gap=8):
    sys.stdout.reconfigure(line_buffering=True)
    seen = Counter()
    last = {}
    for k in range(n):
        ne, holders, arrs = sample()
        print(f"采样{k + 1}: 实体 {ne}，候选数组 {len(arrs)}，持有者 {len(holders)}")
        for h, v in holders.items():
            seen[h] += 1
            last[h] = v
        time.sleep(gap)
    print("== 每次都出现的持有者（实体列表候选）:")
    for h, c in seen.most_common(15):
        a, off, nxt, lib = last[h]
        print(f"  {h} 出现 {c}/{n} 次 -> 数组 {a} (-0x{off:x}) 后续i32 [{nxt}]{' [libg]' if lib else ''}")


if __name__ == "__main__":
    main(*(int(x) for x in sys.argv[1:3]))
