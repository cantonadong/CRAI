"""进入一局对战（训练营或天梯），1080x1920 竖屏坐标。
流程：识别界面并回到主界面 -> 点击入口 -> 用常驻读取器等待新对局出现。
用法: python scripts/start_training.py [training|ladder]
"""
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.reader.stream import LiveReader  # noqa: E402
from ui import go_home, tap  # noqa: E402

MENU = (983, 203)            # 主界面右上角菜单
TRAINING = (647, 628)        # 菜单"训练营"
CONFIRM = (736, 1110)        # 弹窗"确定"
LADDER = (540, 1500)         # 主界面"对战"（天梯，会改变奖杯）


def main(mode: str = "training"):
    if not go_home():
        print("无法回到主界面"); return 1
    live = LiveReader(period_ms=100, rescan=0)
    try:
        n, old = live.latest(timeout=10)
        old_roster = old.roster if old.battle else ""
        old_seq = sum(p.seq for p in old.players) if old.battle and old.players else -1
        if mode == "ladder":
            tap(*LADDER)
            wait = 90                  # 匹配真人可能较久
        else:
            tap(*MENU); time.sleep(1.5)
            tap(*TRAINING); time.sleep(1.5)
            tap(*CONFIRM)
            wait = 30
        t0 = time.time()
        while time.time() - t0 < wait:
            n, f = live.latest(newer_than=n, timeout=5)
            # 新对局：对局对象地址变化，或出牌序号回落（新对局可能复用旧对象的地址）
            is_new = f.roster != old_roster or sum(p.seq for p in f.players) < old_seq or old_seq <= 0
            if f.battle and f.players and is_new and all(p.king_hp[0] > 0 for p in f.players):
                print(f"已进入对战（{mode}）"); return 0
    finally:
        live.close()
    print("超时未检测到对战"); return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "training"))
