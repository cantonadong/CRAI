"""训练策略网络 v2（模仿学习）。数据：data/il_v2/*.npz（crai/data/dataset_v2.py 生成）。

- 按对局划分训练/验证（默认 5% 对局做验证）。
- "不出"样本很多：每轮随机只取 keep_noop 比例，并按倒数加权，保持期望不变。
- 只学结局与原局一致的对局，或不一致对局的偏离前部分（数据集已截断）。
- 输出 models/hog26_v2.pt，并打印验证集指标：出牌/不出准确率、出哪张的准确率、落点 top1/邻近 1 格准确率。

用法：python -u scripts/train_v2.py [轮数 20] [keep_noop 0.15] [load_noop 0.3]
"""
import glob
import json
import sys
import time
from pathlib import Path

import numpy as np
import torch
import torch.nn.functional as F

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.model.policy_v2 import PolicyV2  # noqa: E402


def load(files, noop_keep: float = 1.0, seed: int = 0):
    """载入并拼接。noop_keep < 1 时只随机保留这一比例的"不出"样本（出牌/技能全部保留），
    全量约 1 千万决策点，全部载入约 7GB，下采样后约三分之一。先数行数再预分配，避免拼接时内存翻倍。"""
    rng = np.random.default_rng(seed)
    keeps, n = [], 0
    for f in files:
        lab = np.load(f)["label"]
        k = (lab[:, 0] > 0) | (rng.random(len(lab)) < noop_keep)
        keeps.append(k)
        n += int(k.sum())
    S = np.empty((n, 14), np.float16)
    C = np.empty((n, 10), np.int16)
    E = None
    L = np.empty((n, 4), np.int64)
    i = 0
    for f, k in zip(files, keeps):
        d = np.load(f)
        m = int(k.sum())
        if E is None:
            E = np.empty((n,) + d["ents"].shape[1:], np.float16)
        S[i:i + m] = d["scal"][k]; C[i:i + m] = d["cards"][k]; E[i:i + m] = d["ents"][k]; L[i:i + m] = d["label"][k]
        i += m
    return torch.from_numpy(S), torch.from_numpy(C), torch.from_numpy(E), torch.from_numpy(L)


def losses(model, s, c, e, l, w_noop):
    typ, slot, pos = l[:, 0], l[:, 1], l[:, 3] * 18 + l[:, 2]
    tl, sl, pl = model(s, c, e, slot=slot.clamp(min=0))
    w = torch.where(typ == 0, torch.full_like(s[:, 0], w_noop), torch.ones_like(s[:, 0]))
    loss_t = (F.cross_entropy(tl, typ, reduction="none") * w).sum() / w.sum()
    m = typ == 1
    loss_s = F.cross_entropy(sl[m], slot[m]) if m.any() else tl.sum() * 0
    loss_p = F.cross_entropy(pl[m], pos[m]) if m.any() else tl.sum() * 0
    return loss_t, loss_s, loss_p, (tl, sl, pl, typ, slot, pos, m)


@torch.no_grad()
def evaluate(model, data, dev, bs=4096):
    model.eval()
    s, c, e, l = data
    tot = {"n": 0, "play_n": 0, "slot_ok": 0, "pos_ok": 0, "pos_near": 0}
    probs, truth = [], []
    for i in range(0, len(l), bs):
        b = [t[i:i + bs].to(dev) for t in (s, c, e, l)]
        b[0], b[2] = b[0].float(), b[2].float()
        _, _, _, (tl, sl, pl, typ, slot, pos, m) = losses(model, *b, 1.0)
        probs.append(torch.softmax(tl.float(), -1)[:, 1].cpu())
        truth.append(m.cpu())
        tot["n"] += len(typ)
        tot["play_n"] += m.sum().item()
        if m.any():
            tot["slot_ok"] += (sl[m].argmax(-1) == slot[m]).sum().item()
            pp = pl[m].argmax(-1)
            tot["pos_ok"] += (pp == pos[m]).sum().item()
            dx = (pp % 18 - pos[m] % 18).abs()
            dy = (pp // 18 - pos[m] // 18).abs()
            tot["pos_near"] += ((dx <= 1) & (dy <= 1)).sum().item()
    model.train()
    pn = max(tot["play_n"], 1)
    p, y = torch.cat(probs), torch.cat(truth)
    # 出牌时机：阈值取"预测出牌比例 = 真实比例"，此时准确率 = 召回率；另给 AUC
    k = int(y.sum())
    thr = p.topk(k).values[-1] if k > 0 else 1.0
    hit = ((p >= thr) & y).sum().item() / max(k, 1)
    order = p.argsort()
    ranks = torch.empty_like(order, dtype=torch.float)
    ranks[order] = torch.arange(1, len(p) + 1, dtype=torch.float)
    npos, nneg = k, len(p) - k
    auc = ((ranks[y].sum() - npos * (npos + 1) / 2) / max(npos * nneg, 1)).item()
    return {"play_auc": auc, "play_hit@rate": hit, "slot_acc": tot["slot_ok"] / pn,
            "pos_top1": tot["pos_ok"] / pn, "pos_within1": tot["pos_near"] / pn}


def main(epochs="20", keep_noop="0.15", load_noop="0.3"):
    """keep_noop：每轮从已载入的"不出"样本里再取的比例；load_noop：载入时保留的"不出"比例。
    "不出"样本的总权重 = 1 / (keep_noop × load_noop)，保持期望不变。"""
    epochs, keep_noop, load_noop = int(epochs), float(keep_noop), float(load_noop)
    files = sorted(glob.glob(str(ROOT / "data/il_v2/*.npz")))
    rng = np.random.default_rng(0)
    rng.shuffle(files)
    n_val = max(1, len(files) // 20)
    val, train = load(files[:n_val]), load(files[n_val:], noop_keep=load_noop)
    print(f"对局 训练 {len(files) - n_val} / 验证 {n_val}；样本 训练 {len(train[3])} / 验证 {len(val[3])}", flush=True)
    dev = "cuda"
    model = PolicyV2().to(dev)
    opt = torch.optim.AdamW(model.parameters(), lr=3e-4, weight_decay=0.01)
    typ = train[3][:, 0]
    act_idx = torch.nonzero(typ > 0).squeeze(1)
    noop_idx = torch.nonzero(typ == 0).squeeze(1)
    bs, best = 512, None
    sched = torch.optim.lr_scheduler.OneCycleLR(opt, 3e-4, total_steps=epochs * ((len(act_idx) + int(len(noop_idx) * keep_noop)) // bs + 1))
    for ep in range(epochs):
        t0 = time.time()
        sub = noop_idx[torch.randperm(len(noop_idx))[: int(len(noop_idx) * keep_noop)]]
        idx = torch.cat([act_idx, sub])[torch.randperm(len(act_idx) + len(sub))]
        agg = np.zeros(3)
        for i in range(0, len(idx), bs):
            j = idx[i:i + bs]
            s, c, e, l = (t[j].to(dev, non_blocking=True) for t in train)
            with torch.autocast("cuda", dtype=torch.bfloat16):
                lt, ls, lp, _ = losses(model, s.float(), c, e.float(), l, 1.0 / (keep_noop * load_noop))
                loss = lt + ls + lp
            opt.zero_grad(set_to_none=True)
            loss.backward()
            torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0)
            opt.step()
            sched.step()
            agg += [lt.item(), ls.item(), lp.item()]
        nb = max(1, len(idx) // bs)
        m = evaluate(model, val, dev)
        print(f"epoch {ep + 1:2d} loss type {agg[0] / nb:.3f} slot {agg[1] / nb:.3f} pos {agg[2] / nb:.3f} | "
              + " ".join(f"{k} {v:.3f}" for k, v in m.items()) + f" | {time.time() - t0:.0f}s", flush=True)
        score = m["slot_acc"] + m["pos_within1"] + m["play_auc"]
        if best is None or score > best:
            best = score
            out = ROOT / "models/hog26_v2.pt"
            out.parent.mkdir(exist_ok=True)
            torch.save({"state": model.state_dict(), "metrics": m, "epoch": ep + 1,
                        "vocab": json.loads((ROOT / "data/il_v2/vocab.json").read_text())}, out)
    # 实战出牌阈值：验证集上让"预测出牌比例 = 真实出牌比例"的概率值，存进模型文件供 v2 bot 读取
    out = ROOT / "models/hog26_v2.pt"
    ck = torch.load(out, map_location="cpu")
    model.load_state_dict(ck["state"])
    model.eval()
    s, c, e, l = val
    ps = []
    with torch.no_grad():
        for i in range(0, len(l), 4096):
            tl, _, _ = model(s[i:i + 4096].to(dev).float(), c[i:i + 4096].to(dev), e[i:i + 4096].to(dev).float())
            ps.append(torch.softmax(tl.float(), -1)[:, 1].cpu())
    p = torch.cat(ps)
    rate = (l[:, 0] == 1).float().mean().item()
    ck["threshold"] = float(p.quantile(1 - rate))
    torch.save(ck, out)
    print(f"最佳模型（第 {ck['epoch']} 轮）已保存到 models/hog26_v2.pt；实战出牌阈值 {ck['threshold']:.3f}"
          f"（验证集真实出牌比例 {rate:.3f}）", flush=True)


if __name__ == "__main__":
    main(*sys.argv[1:])
