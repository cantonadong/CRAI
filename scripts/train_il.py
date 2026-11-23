"""训练模仿学习策略。按对局划分训练/验证集，输出验证指标并保存模型到 models/hog26_il.pt
用法: python scripts/train_il.py [轮数=60]
"""
import sys
from pathlib import Path

import numpy as np
import torch
import torch.nn.functional as F

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.model.policy import Policy  # noqa: E402


def main(epochs=60, val_games=4, seed=0):
    torch.manual_seed(seed); np.random.seed(seed)
    d = np.load(ROOT / "data" / "hog26_il.npz")
    num, ev, lab, w, game = (torch.tensor(d[k]) for k in ("num", "ev", "label", "weight", "game"))
    games = np.unique(d["game"])
    rng = np.random.default_rng(seed)
    val_g = set(rng.choice(games, size=val_games, replace=False).tolist())
    is_val = torch.tensor([g in val_g for g in d["game"]])
    dev = "cuda" if torch.cuda.is_available() else "cpu"
    num, ev, lab, w = num.to(dev), ev.to(dev), lab.to(dev), w.to(dev)
    tr, va = (~is_val).nonzero().squeeze(1).to(dev), is_val.nonzero().squeeze(1).to(dev)
    print(f"设备 {dev}；训练对局 {len(games) - val_games}，验证对局 {val_games}；训练样本 {len(tr)}，验证样本 {len(va)}")

    model = Policy().to(dev)
    opt = torch.optim.AdamW(model.parameters(), lr=2e-3, weight_decay=1e-3)
    pos_rate = (lab[tr, 0] == 1).float().mean()
    pos_w = (1 - pos_rate) / pos_rate

    def loss_on(idx):
        n, e, l, ww = num[idx], ev[idx], lab[idx], w[idx]
        play_l, card_l, x_l, y_l = model(n, e, card=l[:, 1])
        lp = F.binary_cross_entropy_with_logits(play_l, (l[:, 0] == 1).float(), pos_weight=pos_w, reduction="none")
        loss = (lp * ww).mean()
        m = l[:, 0] == 1
        if m.any():
            lc = F.cross_entropy(card_l[m], l[m, 1], reduction="none")
            lx = F.cross_entropy(x_l[m], l[m, 2], reduction="none")
            ly = F.cross_entropy(y_l[m], l[m, 3], reduction="none")
            loss = loss + ((lc + 0.5 * lx + 0.5 * ly) * ww[m]).mean()
        return loss

    best, best_state = 1e9, None
    for ep in range(epochs):
        model.train()
        perm = tr[torch.randperm(len(tr), device=dev)]
        for i in range(0, len(perm), 256):
            opt.zero_grad(); loss_on(perm[i:i + 256]).backward(); opt.step()
        model.eval()
        with torch.no_grad():
            vl = loss_on(va).item()
        if vl < best:
            best, best_state = vl, {k: v.clone() for k, v in model.state_dict().items()}
        if ep % 10 == 0 or ep == epochs - 1:
            print(f"轮 {ep:3d} 验证损失 {vl:.3f}")
    model.load_state_dict(best_state); model.eval()

    # ---- 验证指标 ----
    with torch.no_grad():
        n, e, l = num[va], ev[va], lab[va]
        play_l, card_l, _, _ = model(n, e)
        m = l[:, 0] == 1
        _, _, x_l, y_l = model(n[m], e[m], card=l[m, 1])
        prob = torch.sigmoid(play_l)
        # 出牌时机：AUC
        pos, neg = prob[m], prob[~m]
        auc = (pos.unsqueeze(1) > neg.unsqueeze(0)).float().mean().item()
        top1 = (card_l[m].argmax(-1) == l[m, 1]).float().mean().item()
        top3 = (card_l[m].topk(3, -1).indices == l[m, 1:2]).any(-1).float().mean().item()
        dx = (x_l.argmax(-1) - l[m, 2]).abs().float()
        dy = (y_l.argmax(-1) - l[m, 3]).abs().float()
        # 基线：只按"可用牌中训练集最常出的牌"选
        freq = torch.bincount(lab[tr][lab[tr, 0] == 1, 1], minlength=8).float()
        avail = n[m][:, 6:14] > 0.5
        base = (freq.unsqueeze(0).expand(avail.shape[0], 8).masked_fill(~avail, -1)).argmax(-1)
        base1 = (base == l[m, 1]).float().mean().item()
    print(f"验证（{int(m.sum())} 次出牌）: 时机 AUC {auc:.3f} | 选牌 top1 {top1:.1%} top3 {top3:.1%}（频率基线 top1 {base1:.1%}）"
          f" | 落点误差 中位 |dx| {dx.median():.0f} 格 |dy| {dy.median():.0f} 格，同格 {((dx == 0) & (dy == 0)).float().mean():.1%}，"
          f"±1 格内 {((dx <= 1) & (dy <= 1)).float().mean():.1%}")
    out = ROOT / "models"; out.mkdir(exist_ok=True)
    torch.save({"state": model.state_dict(), "deck": [str(s) for s in d["deck"]]}, out / "hog26_il.pt")
    print("模型 ->", out / "hog26_il.pt")


if __name__ == "__main__":
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 60)
