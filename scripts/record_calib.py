"""被动标定：用户正常对战，同时记录触摸事件和内存中新出现的己方单位，配对后拟合屏幕<->竞技场映射。

- 触摸：adb getevent 读 /dev/input/event2（LDPlayer 的 "input" 设备，原始坐标横屏 0..1919 x 0..1079）
- 单位：cr_reader 实时流；己方新实体第一次出现的位置 = 落点（部署动画期间不动）
- 配对：每个新实体匹配它出现前 2.5 秒内、最近一次触摸抬起的位置
- 原始触摸坐标到竖屏像素的旋转方式未知：4 种候选都拟合，取残差最小的
结果写入 configs/screen_1080x1920.json

用法: .venv\\Scripts\\python.exe -u scripts\\record_calib.py [秒数=150]
"""
import json
import re
import subprocess
import sys
import threading
import time
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "scripts"))
from crai.reader.stream import ADB, SERIAL, stream  # noqa: E402
from calibrate import apply_h, fit_homography  # noqa: E402

TOUCH_DEV = "/dev/input/event2"
MULTI_UNIT = {26000010}  # 骷髅兵：多个单位散开，不作为样本
EV = re.compile(r"(ABS_MT_POSITION_X|ABS_MT_POSITION_Y|BTN_TOUCH|ABS_MT_TRACKING_ID)\s+([0-9a-fA-F]+|UP|DOWN)")

touch_ups = []  # (host_time, raw_x, raw_y)


def touch_thread(stop):
    p = subprocess.Popen([ADB, "-s", SERIAL, "shell", f"getevent -l {TOUCH_DEV}"], stdout=subprocess.PIPE, text=True, bufsize=1)
    x = y = None
    for line in p.stdout:
        if stop.is_set():
            break
        m = EV.search(line)
        if not m:
            continue
        key, val = m.groups()
        if key == "ABS_MT_POSITION_X":
            x = int(val, 16)
        elif key == "ABS_MT_POSITION_Y":
            y = int(val, 16)
        elif (key == "BTN_TOUCH" and val == "UP") or (key == "ABS_MT_TRACKING_ID" and val.lower() == "ffffffff"):
            if x is not None and y is not None:
                touch_ups.append((time.time(), x, y))
    p.kill()


ROTATIONS = {  # raw(x 0..1919, y 0..1079) -> 竖屏 (px 0..1079, py 0..1919)
    "px=y,py=x": lambda x, y: (y, x),
    "px=1079-y,py=x": lambda x, y: (1079 - y, x),
    "px=y,py=1919-x": lambda x, y: (y, 1919 - x),
    "px=1079-y,py=1919-x": lambda x, y: (1079 - y, 1919 - x),
}


def main(seconds=150):
    sys.stdout.reconfigure(line_buffering=True)
    stop = threading.Event()
    th = threading.Thread(target=touch_thread, args=(stop,), daemon=True)
    th.start()
    seen, samples, t0 = set(), [], time.time()
    first = True
    for f in stream(period_ms=30, rescan=3):
        if time.time() - t0 > seconds:
            break
        if not f.battle:
            continue
        me = f.players[f.own_index()]
        ids = {e.id for e in f.entities}
        if first:
            seen |= ids; first = False
            print(f"开始记录：己方 side={me.side}，现在请正常对战（单体卡最好放在不同位置）")
            continue
        for e in f.entities:
            if e.id in seen:
                continue
            seen.add(e.id)
            if e.side != me.side or e.card in MULTI_UNIT or e.card == -1:
                continue
            now = time.time()
            cands = [t for t in touch_ups if now - 2.5 <= t[0] <= now]
            if not cands:
                print(f"  新单位 card={e.card} ({e.x},{e.y}) 但没有匹配到触摸")
                continue
            t = cands[-1]
            samples.append((t[1], t[2], e.x, e.y, e.card))
            print(f"  样本{len(samples)}: 触摸原始({t[1]},{t[2]}) -> 竞技场({e.x},{e.y}) card={e.card}")
    stop.set()
    if len(samples) < 5:
        sys.exit(f"样本不足（{len(samples)}）")
    src = np.array([[s[2], s[3]] for s in samples], float)
    best = None
    for name, rot in ROTATIONS.items():
        dst = np.array([rot(s[0], s[1]) for s in samples], float)
        H = fit_homography(src, dst)
        res = np.array([np.hypot(*(np.array(apply_h(H, *p)) - q)) for p, q in zip(src, dst)])
        print(f"旋转 {name}: 残差中位 {np.median(res):.1f}px 最大 {res.max():.1f}px")
        if best is None or np.median(res) < best[0]:
            best = (float(np.median(res)), name, H, res)
    med, name, H, res = best
    cfg = {"screen": [1080, 1920], "model": "homography arena(X,Y)->screen(px,py); 己方在下方",
           "touch_rotation": name, "H": H.tolist(),
           "hand_slots_px": [[337, 1710], [540, 1710], [742, 1710], [945, 1710]],
           "samples_raw_touch": [list(map(int, s)) for s in samples], "residual_px_median": med}
    out = ROOT / "configs" / "screen_1080x1920.json"
    out.write_text(json.dumps(cfg, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"采用 {name}，残差中位 {med:.1f}px -> {out}")


if __name__ == "__main__":
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 150)
