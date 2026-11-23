"""简单的模拟器 UI 操作：截图、点击、拖拽。

用法:
  python scripts/ui.py shot [文件名]          截图到 re/ui/<文件名>.png
  python scripts/ui.py tap x y               点击
  python scripts/ui.py swipe x1 y1 x2 y2 ms  拖拽
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.device import ADB, SERIAL  # noqa: E402

OUT = ROOT / "re" / "ui"


def adb(*a):
    return subprocess.run([ADB, "-s", SERIAL, *a], capture_output=True, text=True).stdout


def shot(name="screen"):
    OUT.mkdir(parents=True, exist_ok=True)
    adb("shell", "screencap -p /data/local/tmp/ui.png")
    p = OUT / f"{name}.png"
    adb("pull", "/data/local/tmp/ui.png", str(p))
    return p


def tap(x, y):
    adb("shell", f"input tap {x} {y}")


def back():
    adb("shell", "input keyevent 4")


def _near(c, ref, tol=30):
    return all(abs(a - b) <= tol for a, b in zip(c, ref))


def screen_state() -> str:
    """识别当前界面：home / result / unknown。
    特征点避开底部 su 提示框（约 x 220-860, y 1700-1845）。"""
    from PIL import Image
    im = Image.open(shot("_state")).convert("RGB")
    if _near(im.getpixel((540, 1530)), (255, 187, 0)):
        return "home"
    if _near(im.getpixel((110, 1110)), (41, 103, 186)) and _near(im.getpixel((110, 560)), (161, 14, 70)):
        return "result"
    return "unknown"


def go_home(max_steps: int = 8) -> bool:
    """从结算界面/弹窗/其他页面回到主界面。"""
    import time
    for _ in range(max_steps):
        s = screen_state()
        if s == "home":
            return True
        if s == "result":
            tap(540, 1745); time.sleep(3)
            continue
        # 未知界面：先试底部"确定"，再按返回键
        tap(540, 1850); time.sleep(1.5)
        if screen_state() == "home":
            return True
        back(); time.sleep(1.5)
    return screen_state() == "home"


def swipe(x1, y1, x2, y2, ms=300):
    adb("shell", f"input swipe {x1} {y1} {x2} {y2} {ms}")


if __name__ == "__main__":
    cmd, *args = sys.argv[1:]
    if cmd == "shot":
        print(shot(*args))
    elif cmd == "tap":
        tap(*map(int, args))
    elif cmd == "swipe":
        swipe(*map(int, args))
