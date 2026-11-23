"""单步调试下牌：每一步截图 + 读内存，判断点击是否生效。
用法: python -u scripts/debug_deploy.py <模式> <槽位> <x> <y>
  模式 tap:   点槽位 -> 等待 -> 点场地
  模式 drag:  从槽位拖到场地
"""
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.reader.stream import snapshot  # noqa: E402
from ui import shot, swipe, tap  # noqa: E402

HAND = [(337, 1710), (540, 1710), (742, 1710), (945, 1710)]


def state(tag):
    f = snapshot()
    me = f.players[f.own_index()]
    print(f"[{tag}] 圣水={me.elixir:.2f} seq={me.seq} 手牌={me.hand_cards()} 己方实体={sum(e.side == me.side for e in f.entities)}")
    return me


def main(mode, slot, x, y, gap=0.4):
    sys.stdout.reconfigure(line_buffering=True)
    before = state("前")
    sx, sy = HAND[slot]
    if mode == "tap":
        tap(sx, sy); time.sleep(gap); shot("d1_selected")
        tap(x, y)
    else:
        swipe(sx, sy, x, y, 350)
    time.sleep(0.3); shot("d2_after")
    time.sleep(1.5)
    after = state("后")
    print("结果:", "成功" if after.seq > before.seq else "失败（出牌序号没变）")


if __name__ == "__main__":
    m, s, x, y = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
    main(m, s, x, y)
