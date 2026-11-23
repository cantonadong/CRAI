"""在我们的 libg.so 里搜索 FirstLight_CR probe 记录的函数开头字节，粗看哪些函数能直接定位。

用法：python scripts/fl_prologue_scan.py
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "refs/FirstLight_CR/native_runner/probe"
LIBG = ROOT / "apk/libs/arm64-v8a/libg.so"

data = LIBG.read_bytes()
pat = re.compile(r"k(\w+?)Prologue\[\d+\]\s*=\s*\{([^}]*)\}", re.S)
offs = re.compile(r"k(\w+?)Offset\s*=\s*(0x[0-9a-fA-F]+)")

found = {}
for f in sorted(SRC.glob("*")):
    if f.suffix not in {".cpp", ".h", ".inc"}:
        continue
    text = f.read_text(encoding="utf-8", errors="ignore")
    offsets = dict(offs.findall(text))
    for name, body in pat.findall(text):
        bs = bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", body))
        if name in found:
            continue
        hits = [m.start() for m in re.finditer(re.escape(bs), data)]
        # 只看 4 字节对齐的命中（ARM64 指令对齐）
        hits = [h for h in hits if h % 4 == 0]
        found[name] = (offsets.get(name, "?"), len(bs), hits)

for name, (old, n, hits) in found.items():
    shown = ", ".join(hex(h) for h in hits[:4]) + (" ..." if len(hits) > 4 else "")
    print(f"{name:55s} old={old:>10s} len={n:2d} hits={len(hits):3d} {shown}")
