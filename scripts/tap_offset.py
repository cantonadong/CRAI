"""估计"点击下牌"相对"拖拽下牌"标定模型的像素偏移。

点击样本：(点击像素, 实际落点)。用拖拽模型算出"实际落点格子中心"应在的像素，
与点击像素之差的中位数即为点击方式的修正量（点击时应额外加上 -偏移）。
用法: python scripts/tap_offset.py  （样本在 re/calib_samples_tap.json）
"""
import json
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.client.control import Screen  # noqa: E402

scr = Screen()
s = json.loads((ROOT / "re" / "calib_samples_tap.json").read_text(encoding="utf-8"))
d = []
for px, py, X, Y in s:
    ex, ey = scr.arena_to_px((X // 1000) * 1000 + 500, (Y // 1000) * 1000 + 500)
    d.append((ex - px, ey - py))
d = np.array(d, float)
print("每个样本 (拖拽模型像素 - 点击像素):", d.astype(int).tolist())
dx, dy = np.median(d, axis=0)
print(f"中位偏移 dx={dx:.1f} dy={dy:.1f}  -> 点击时目标像素应加上 ({-dx:.0f}, {-dy:.0f})")
cfg_path = ROOT / "configs" / "screen_1080x1920.json"
cfg = json.loads(cfg_path.read_text(encoding="utf-8"))
cfg["tap_offset_px"] = [round(-dx), round(-dy)]
cfg_path.write_text(json.dumps(cfg, indent=2, ensure_ascii=False), encoding="utf-8")
print("已写入 configs/screen_1080x1920.json: tap_offset_px")
