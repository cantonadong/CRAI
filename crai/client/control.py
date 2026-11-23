"""下牌控制：竞技场格子 -> 屏幕像素 -> adb 拖拽。

坐标约定：
  - 竞技场坐标 (X, Y) 为内存单位，X 0..18000（18 格），Y 0..32000（32 格），1 格 = 1000
  - 标定数据来自 己方 side=1（国王塔在 Y=29000，屏幕下方）的对局
  - 若己方国王塔在 Y<16000（屏幕依然把己方画在下方），先做 180° 翻转再映射（待正式对战验证，见 docs/re_notes.md）
"""

from __future__ import annotations

import json
import subprocess
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
CFG_PATH = ROOT / "configs" / "screen_1080x1920.json"
from crai.device import ADB, SERIAL  # noqa: E402


def _quad_feats(X, Y):
    x, y = X / 1000.0, Y / 1000.0
    return np.array([1, x, y, x * x, x * y, y * y])


class Screen:
    def __init__(self, cfg_path: Path = CFG_PATH):
        cfg = json.loads(cfg_path.read_text(encoding="utf-8"))
        self.w, self.h = cfg["screen"]
        self.model = cfg["model"]
        self.params = cfg["params"]
        self.hand_slots = [tuple(p) for p in cfg["hand_slots_px"]]

    def arena_to_px(self, X: float, Y: float, own_at_top: bool = False) -> tuple[int, int]:
        if own_at_top:
            X, Y = 18000 - X, 32000 - Y
        # 画面左右对称（中线 px=540）。二次拟合的标定点集中在右半边，左半边偏差大
        # （2026-11-22 实测左桥同一行比右桥低约 40px≈1 格，左桥野猪下在桥后一格、跳不过河）。
        # 所以左半边用右半边的镜像：px = 2*540 - px(18000-X, Y)，py = py(18000-X, Y)。
        if X < 9000 and self.model == "quadratic":
            mx, my = self._fit(18000 - X, Y)
            px, py = 2 * self.w / 2 - mx, my
            return int(min(max(px, 15), self.w - 15)), int(min(max(py, 60), 1540))
        px, py = self._fit(X, Y)
        return int(min(max(px, 15), self.w - 15)), int(min(max(py, 60), 1540))

    def _fit(self, X: float, Y: float) -> tuple[float, float]:
        if self.model == "quadratic":
            f = _quad_feats(X, Y)
            px, py = float(f @ self.params["cx"]), float(f @ self.params["cy"])
        else:
            H = np.array(self.params)
            p = H @ np.array([X, Y, 1.0])
            px, py = p[0] / p[2], p[1] / p[2]
        return px, py

    @staticmethod
    def tile_center(tx: int, ty: int) -> tuple[int, int]:
        return tx * 1000 + 500, ty * 1000 + 500


class TouchD:
    """模拟器里常驻的触摸注入程序（crai/tools/touchd.c，/data/local/tmp/touchd）。
    直接向触摸屏设备写事件，每次点击只需几十毫秒；`adb shell input` 每次要启动 Java 进程，两次点击约 280ms。
    第一次用时启动并保持管道；启动失败（没部署、没 root）时 available=False，调用方退回 input 命令。"""

    _inst = None

    def __init__(self):
        self.proc = None
        try:
            subprocess.run([ADB, "-s", SERIAL, "shell", "su -c 'pkill touchd'"], capture_output=True, timeout=15)
            self.proc = subprocess.Popen([ADB, "-s", SERIAL, "shell", "su -c '/data/local/tmp/touchd /dev/input/event2'"],
                                         stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
            self.proc.stdin.write("P\n")
            self.proc.stdin.flush()
            self.available = self.proc.stdout.readline().strip() == "ok"
        except Exception:  # noqa: BLE001
            self.available = False

    @classmethod
    def get(cls) -> "TouchD":
        if cls._inst is None or cls._inst.proc is None or cls._inst.proc.poll() is not None:
            cls._inst = cls()
        return cls._inst

    def cmd(self, line: str) -> bool:
        try:
            self.proc.stdin.write(line + "\n")
            self.proc.stdin.flush()
            return self.proc.stdout.readline().strip() == "ok"
        except Exception:  # noqa: BLE001
            self.available = False
            return False


def swipe(x1, y1, x2, y2, ms=300):
    t = TouchD.get()
    if t.available and t.cmd(f"W {x1} {y1} {x2} {y2} {ms}"):
        return
    subprocess.run([ADB, "-s", SERIAL, "shell", f"input swipe {x1} {y1} {x2} {y2} {ms}"], check=False)


def tap2(x1, y1, x2, y2):
    """连续点两下（选卡 -> 目标）。优先用常驻触摸程序（约 70ms），否则一次 adb 调用里两次 input tap（约 280ms）。"""
    t = TouchD.get()
    if t.available and t.cmd(f"T {x1} {y1}") and t.cmd(f"T {x2} {y2}"):
        return
    subprocess.run([ADB, "-s", SERIAL, "shell", f"input tap {x1} {y1}; input tap {x2} {y2}"], check=False)


def tap(x, y):
    t = TouchD.get()
    if t.available and t.cmd(f"T {x} {y}"):
        return
    subprocess.run([ADB, "-s", SERIAL, "shell", f"input tap {x} {y}"], check=False)


# 英雄技能按钮（单英雄 HUD，1080×1920 竖屏）。参考 RoyaleHarness ability_calibration.example.json 的 [974, 1450]；
# 实战用"按下后技能次数 1→0"确认（scripts/run_fl_live.py 记录 ability_confirmed）
ABILITY_PX = (974, 1450)


def press_ability():
    tap(*ABILITY_PX)


# 点击一个无副作用的位置以取消选中（"下一张"预览区）
DESELECT_PX = (95, 1700)


def deselect():
    t = TouchD.get()
    if t.available and t.cmd(f"T {DESELECT_PX[0]} {DESELECT_PX[1]}"):
        return
    subprocess.run([ADB, "-s", SERIAL, "shell", f"input tap {DESELECT_PX[0]} {DESELECT_PX[1]}"], check=False)


def deploy(screen: Screen, slot: int, X: float, Y: float, own_at_top: bool = False, mode: str = "tap") -> tuple[int, int]:
    """把第 slot 张手牌（0..3，从左到右）下到竞技场坐标 (X, Y)。返回目标像素。

    mode="tap"：点卡再点目标（默认，较快）；落点非法时卡会保持选中，调用方应在超时后 deselect()。
    mode="drag"：从手牌拖到目标（备用）。
    """
    sx, sy = screen.hand_slots[slot]
    px, py = screen.arena_to_px(X, Y, own_at_top)
    if mode == "drag":
        swipe(sx, sy, px, py)
    else:
        tap2(sx, sy, px, py)
    return px, py
