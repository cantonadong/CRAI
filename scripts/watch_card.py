"""观察某张卡新实体第一次出现的位置（用于验证回放指令修改）。
用法: python -u scripts/watch_card.py <card_id> <期望x> <期望y> <原x> <原y> [超时秒=60]
"""
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.reader.stream import LiveReader  # noqa: E402
from ui import shot  # noqa: E402

card, nx, ny, ox, oy = (int(v) for v in sys.argv[1:6])
timeout = float(sys.argv[6]) if len(sys.argv) > 6 else 60
live = LiveReader(period_ms=30, rescan=1)
seen, n, t0, first = set(), -1, time.time(), True
try:
    while time.time() - t0 < timeout:
        n, f = live.latest(newer_than=n, timeout=5)
        if not f.battle:
            continue
        if first:  # 首帧已有的实体不算
            seen |= {e.id for e in f.entities}; first = False; continue
        for e in f.entities:
            if e.id in seen:
                continue
            seen.add(e.id)
            if e.card == card:
                dn, do = abs(e.x - nx) + abs(e.y - ny), abs(e.x - ox) + abs(e.y - oy)
                print(f"出现 card {card} side {e.side} at ({e.x},{e.y})；距改后 {dn}，距原始 {do}")
                print("结论:", "✅ 修改生效" if dn < do else "❌ 未生效")
                shot("poke_result")
                sys.exit(0)
    print("超时未观察到")
finally:
    live.close()
