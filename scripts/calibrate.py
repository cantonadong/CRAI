"""屏幕 <-> 竞技场坐标 标定 v3（需要在对战中运行，会实际下牌！）

做法：
  - LiveReader 只读最新帧（避免处理慢时读到积压的旧帧）
  - 每个测试点选一张"单体、出场即停在落点"的卡（SINGLE_UNIT），用拖拽下牌（点击选卡有选中/取消切换，不可靠）
  - 记录该卡新实体第一次出现时的内存坐标（部署动画期间不移动）
  - 落点吸附到格子中心，测试点多且分散；剔除被塔挤开的异常样本
  - 拟合单应矩阵 H（画面有透视）：竞技场坐标 (X,Y) -> 屏幕像素 (px,py)
结果写入 configs/screen_1080x1920.json

用法（在 D:\\Dev\\CRAI 下）: .venv\\Scripts\\python.exe -u scripts\\calibrate.py
"""
import json
import subprocess
import sys
import time
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.reader.stream import ADB, SERIAL, LiveReader  # noqa: E402

HAND_SLOTS_PX = [(337, 1710), (540, 1710), (742, 1710), (945, 1710)]
# 单体卡: 卡牌ID -> 费用（野猪 2.6 卡组）
SINGLE_UNIT = {26000030: 1, 26000038: 2, 27000000: 3, 26000014: 4, 26000021: 4}
# 用于循环手牌的非单体卡: 卡牌ID -> (费用, 拖放位置)；法术打到对方半场，骷髅兵放己方中路
CYCLE = {28000011: (2, (540, 500)), 28000000: (4, (540, 400)), 26000010: (1, (540, 1100))}
# 己方半场测试点（像素），多行多列，避开公主塔（约 x 180-360/780-960, y 1150-1330）和国王塔（约 x 420-660, y 1250-1450）
# 只取空旷区域：河与公主塔之间的四行全宽，以及公主塔与国王塔之间、后排两角
TEST_POINTS = [(120, 880), (330, 880), (540, 880), (750, 880), (960, 880),
               (225, 960), (435, 960), (645, 960), (855, 960),
               (120, 1040), (330, 1040), (540, 1040), (750, 1040), (960, 1040),
               (225, 1110), (435, 1110), (645, 1110), (855, 1110),
               (450, 1200), (630, 1200), (40, 1250), (1040, 1250),
               (100, 1480), (980, 1480), (300, 1480), (780, 1480)]


def drag(x1, y1, x2, y2):
    subprocess.run([ADB, "-s", SERIAL, "shell", f"input swipe {x1} {y1} {x2} {y2} 300"], check=False)


def fit_homography(src, dst):
    """src: Nx2 竞技场坐标, dst: Nx2 像素。DLT 最小二乘。"""
    # 归一化（Hartley）：竞技场坐标 /1000，像素 /1000，避免数值病态
    S = np.diag([1e-3, 1e-3, 1.0])
    A = []
    for (X, Y), (u, v) in zip(np.asarray(src, float) * 1e-3, np.asarray(dst, float) * 1e-3):
        A.append([X, Y, 1, 0, 0, 0, -u * X, -u * Y, -u])
        A.append([0, 0, 0, X, Y, 1, -v * X, -v * Y, -v])
    _, _, vt = np.linalg.svd(np.asarray(A, dtype=float))
    Hn = vt[-1].reshape(3, 3)
    H = np.linalg.inv(S) @ Hn @ S
    return H / H[2, 2]


def apply_h(H, X, Y):
    p = H @ np.array([X, Y, 1.0])
    return p[0] / p[2], p[1] / p[2]


def _wait(live, cond, timeout):
    """等待满足条件的新帧，返回该帧或 None。"""
    t, n = time.time(), -1
    while time.time() - t < timeout:
        n, fr = live.latest(newer_than=n)
        if fr.battle and cond(fr):
            return fr
    return None


def collect(max_seconds=180):
    samples = []
    live = LiveReader(period_ms=30, rescan=3)
    points = list(TEST_POINTS)
    t_start, n = time.time(), -1
    try:
        while points and time.time() - t_start < max_seconds:
            n, f = live.latest(newer_than=n)
            if not f.battle:
                print("未在对战中"); break
            me = f.players[f.own_index()]
            hand = me.hand_cards()
            choice = next((i for i, c in enumerate(hand) if c in SINGLE_UNIT and me.elixir >= SINGLE_UNIT[c] + 0.1), None)
            if choice is None:
                # 没有可用单体卡；若单体卡短期内也打不出，就打掉一张循环卡让手牌转起来
                soon = any(c in SINGLE_UNIT and me.elixir + 1.5 >= SINGLE_UNIT[c] for c in hand)
                cyc = next((i for i, c in enumerate(hand) if c in CYCLE and me.elixir >= CYCLE[c][0] + 0.1), None)
                if cyc is not None and not soon:
                    _, (cx, cy) = CYCLE[hand[cyc]]
                    print(f"  循环: 槽位{cyc} 卡={hand[cyc]} -> ({cx},{cy})")
                    seq0, side = me.seq, me.side
                    drag(*HAND_SLOTS_PX[cyc], cx, cy)
                    _wait(live, lambda fr: next(p for p in fr.players if p.side == side).seq > seq0, 3.0)
                time.sleep(0.1)
                continue
            px, py = points.pop(0)
            card, side = hand[choice], me.side
            known = {e.id for e in f.entities}
            print(f"  下牌: 槽位{choice} 卡={card} -> ({px},{py})  圣水={me.elixir:.1f}")
            drag(*HAND_SLOTS_PX[choice], px, py)
            is_new = lambda e: e.card == card and e.side == side and e.id not in known  # noqa: E731
            fr = _wait(live, lambda fr: any(is_new(e) for e in fr.entities), 4.0)
            if fr is None:
                print(f"  ({px},{py}) 超时，未检测到新实体")
                continue
            e = next(e for e in fr.entities if is_new(e))
            samples.append((px, py, e.x, e.y))
            print(f"  样本{len(samples)}: ({px},{py}) -> ({e.x},{e.y})")
    finally:
        live.close()
    return samples


def fit_and_save(samples):
    if len(samples) < 6:
        sys.exit(f"样本不足（{len(samples)}），无法拟合")
    if len({s[1] for s in samples}) < 3:
        sys.exit("样本只覆盖不到 3 行，y 方向无法可靠拟合")
    src = np.array([[s[2], s[3]] for s in samples], float)
    dst = np.array([[s[0], s[1]] for s in samples], float)
    H = fit_homography(src, dst)
    for _ in range(2):  # 剔除被塔挤开等异常样本后重拟合
        res = np.array([np.hypot(*(np.array(apply_h(H, *p)) - q)) for p, q in zip(src, dst)])
        keep = res < max(45.0, np.median(res) * 3)
        if keep.all() or keep.sum() < 6:
            break
        print("剔除样本:", [tuple(map(int, (*dst[i], *src[i]))) for i in np.where(~keep)[0]])
        src, dst = src[keep], dst[keep]
        H = fit_homography(src, dst)
    res = [float(np.hypot(*(np.array(apply_h(H, *p)) - q))) for p, q in zip(src, dst)]
    print(f"残差(px): 中位 {np.median(res):.1f}  最大 {max(res):.1f}  （落点吸附到格子中心，<=30px 属正常）")
    cfg = {"screen": [1080, 1920], "model": "homography arena(X,Y)->screen(px,py); 己方在下方",
           "H": H.tolist(), "hand_slots_px": HAND_SLOTS_PX, "deploy": "input swipe 从手牌槽拖到目标点",
           "samples": [list(map(int, s)) for s in samples], "residual_px_median": float(np.median(res))}
    out = ROOT / "configs" / "screen_1080x1920.json"
    out.write_text(json.dumps(cfg, indent=2, ensure_ascii=False), encoding="utf-8")
    print("->", out)


def main():
    sys.stdout.reconfigure(line_buffering=True)
    path = ROOT / "re" / "calib_samples.json"
    old = json.loads(path.read_text(encoding="utf-8")) if path.exists() else []
    samples = old + [list(s) for s in collect()]
    path.write_text(json.dumps(samples), encoding="utf-8")
    print(f"累计样本 {len(samples)}（本次新增 {len(samples) - len(old)}），用 scripts/fit_calib.py 拟合")


if __name__ == "__main__":
    main()
