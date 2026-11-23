"""离线拟合 re/calib_samples.json -> configs/screen_1080x1920.json

- 只用落在格子中心的样本（X%1000、Y%1000 接近 500；排除出场即移动的单位）
- 候选模型：单应矩阵（8 参数）、二次多项式（每轴 6 参数）
- 留一法交叉验证比较预测误差，选更好的；再用迭代剔除（误差 > 2.5×中位且 > 40px）去掉被塔挤开的样本
输出模型用于 竞技场(X,Y) -> 屏幕(px,py)
"""
import json
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
from calibrate import HAND_SLOTS_PX, apply_h, fit_homography  # noqa: E402


def centered(v):
    return abs(v % 1000 - 500) <= 2


def quad_feats(X, Y):
    x, y = X / 1000.0, Y / 1000.0
    return np.array([1, x, y, x * x, x * y, y * y])


def fit_quad(src, dst):
    A = np.array([quad_feats(*p) for p in src])
    cx, *_ = np.linalg.lstsq(A, dst[:, 0], rcond=None)
    cy, *_ = np.linalg.lstsq(A, dst[:, 1], rcond=None)
    return {"cx": cx.tolist(), "cy": cy.tolist()}


def apply_quad(m, X, Y):
    f = quad_feats(X, Y)
    return float(f @ m["cx"]), float(f @ m["cy"])


MODELS = {
    "homography": (lambda s, d: fit_homography(s, d), lambda m, X, Y: apply_h(m, X, Y)),
    "quadratic": (fit_quad, apply_quad),
}


def errors(model, src, dst):
    fit, app = MODELS[model]
    m = fit(src, dst)
    return m, np.array([np.hypot(*(np.array(app(m, *p)) - q)) for p, q in zip(src, dst)])


def loo(model, src, dst):
    fit, app = MODELS[model]
    out = []
    for i in range(len(src)):
        k = np.arange(len(src)) != i
        m = fit(src[k], dst[k])
        out.append(np.hypot(*(np.array(app(m, *src[i])) - dst[i])))
    return np.array(out)


def main():
    samples = json.loads((ROOT / "re" / "calib_samples.json").read_text(encoding="utf-8"))
    good = [s for s in samples if centered(s[2]) and centered(s[3])]
    src = np.array([[s[2], s[3]] for s in good], float)
    dst = np.array([[s[0], s[1]] for s in good], float)
    keep = np.ones(len(good), bool)
    for _ in range(5):  # 用单应模型做异常剔除（参数少，更稳）
        _, e = errors("homography", src[keep], dst[keep])
        med = np.median(e)
        bad = (e > max(40.0, 2.5 * med))
        if not bad.any():
            break
        idx = np.where(keep)[0][np.argmax(e)]
        print(f"剔除: 像素({good[idx][0]},{good[idx][1]}) 竞技场({good[idx][2]},{good[idx][3]}) 误差 {e.max():.0f}px")
        keep[idx] = False
    s, d = src[keep], dst[keep]
    print(f"样本 {len(samples)}，格子中心 {len(good)}，用于拟合 {keep.sum()}")
    best = None
    for name in MODELS:
        l = loo(name, s, d)
        print(f"  {name:10s} 留一误差: 中位 {np.median(l):.1f}px  90% {np.percentile(l, 90):.1f}px  最大 {l.max():.1f}px")
        if best is None or np.median(l) < best[1]:
            best = (name, float(np.median(l)))
    name = best[0]
    m, e = errors(name, s, d)
    app = MODELS[name][1]
    print(f"采用 {name}；拟合残差中位 {np.median(e):.1f}px")
    for nm, (X, Y) in {"左桥前": (3500, 17500), "右桥前": (14500, 17500), "国王塔前": (9000, 26500),
                       "左下角": (500, 31500), "右下角": (17500, 31500), "己方左公主塔": (3500, 25500)}.items():
        px, py = app(m, X, Y)
        print(f"  {nm} ({X},{Y}) -> 像素 ({px:.0f},{py:.0f})")
    cfg = {"screen": [1080, 1920], "model": name,
           "params": m.tolist() if name == "homography" else m,
           "note": "竞技场(X,Y)->屏幕(px,py)，己方在下方；落点吸附到格子中心",
           "hand_slots_px": HAND_SLOTS_PX, "deploy": "input swipe 从手牌槽拖到目标点",
           "loo_median_px": best[1], "samples_used": int(keep.sum())}
    (ROOT / "configs" / "screen_1080x1920.json").write_text(json.dumps(cfg, indent=2, ensure_ascii=False), encoding="utf-8")
    print("-> configs/screen_1080x1920.json")


if __name__ == "__main__":
    main()
