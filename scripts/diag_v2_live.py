"""对比 v2 bot 实战决策日志与训练数据的特征分布，找出实战出牌概率偏低的原因。

用法：python scripts/diag_v2_live.py [决策日志，缺省取最新的 data/bot_runs/v2_decisions_*.jsonl]
"""
import glob
import json
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
SCAL = ["时间", "阶段1", "阶段2", "阶段3", "己方圣水", "对方圣水", "己塔左", "己塔右", "己国王", "敌塔左", "敌塔右",
        "敌国王", "己方出牌数", "对方出牌数"]


def main(path=None):
    path = path or sorted(glob.glob(str(ROOT / "data/bot_runs/v2_decisions_*.jsonl")))[-1]
    live = [json.loads(l) for l in open(path, encoding="utf-8")]
    print(f"实战决策 {len(live)} 次：{path}")
    ls = np.array([d["scal"] for d in live])
    lc = np.array([d["cards"] for d in live])
    lp = np.array([d["p"][1] for d in live])
    le = [np.array(d["ents"]) for d in live]

    files = sorted(glob.glob(str(ROOT / "data/il_v2/*.npz")))[:200]
    S, C, E = [], [], []
    for f in files:
        d = np.load(f)
        S.append(d["scal"].astype(np.float32)); C.append(d["cards"]); E.append(d["ents"].astype(np.float32))
    ts, tc, te = np.concatenate(S), np.concatenate(C), np.concatenate(E)

    print("\n标量特征（均值 实战 / 训练，最小~最大 实战 / 训练）：")
    for i, n in enumerate(SCAL):
        print(f"  {n:6s} {ls[:, i].mean():7.3f} / {ts[:, i].mean():7.3f}   "
              f"{ls[:, i].min():.2f}~{ls[:, i].max():.2f} / {ts[:, i].min():.2f}~{ts[:, i].max():.2f}")
    print(f"\n手牌词表下标：实战未知(=1)比例 {(lc == 1).mean():.2%}，训练 {(tc == 1).mean():.2%}")
    print(f"  实战手牌下标样例 {lc[len(lc) // 2].tolist()}，训练样例 {tc[len(tc) // 2].tolist()}")
    n_live = np.array([len(x) for x in le])
    n_train = (te[:, :, -1] > 0.5).sum(1)
    print(f"单位数：实战平均 {n_live.mean():.1f}，训练平均 {n_train.mean():.1f}")
    live_ids = np.concatenate([x[:, 0] for x in le if len(x)]) if any(len(x) for x in le) else np.zeros(0)
    train_ids = te[:, :, 0][te[:, :, -1] > 0.5]
    print(f"单位词表下标：实战未知(=1)比例 {(live_ids == 1).mean():.2%}，训练 {(train_ids == 1).mean():.2%}")
    ent_names = ["是否己方", "x", "y", "血量比例", "log最大血量", "kind/15"]
    for j, n in enumerate(ent_names, start=1):
        lv = np.concatenate([x[:, j] for x in le if len(x)])
        tv = te[:, :, j][te[:, :, -1] > 0.5]
        print(f"  单位{n:8s} 实战均值 {lv.mean():.3f}  训练均值 {tv.mean():.3f}")
    print(f"\n实战出牌概率：均值 {lp.mean():.3f}，最大 {lp.max():.3f}，超过 0.12 的比例 {(lp >= 0.12).mean():.1%}")


if __name__ == "__main__":
    main(*sys.argv[1:])
