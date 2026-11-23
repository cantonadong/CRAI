"""模仿学习策略网络（小模型，适配少量回放数据）。

输入：num [B, NUM_DIM]、ev [B, K, 6]（是否己方, 卡牌词表下标, x, y, 距今/10s, 位置已知）
输出：play_logit [B]，card_logits [B, 8]（不可用的牌被屏蔽），x_logits [B, 18]，y_logits [B, 32]（以卡牌为条件）
"""

from __future__ import annotations

import torch
import torch.nn as nn
import torch.nn.functional as F

from crai.data.dataset import NUM_DIM, VOCAB

AVAIL_SLICE = slice(6, 14)  # num 中的手牌可用掩码位置


class Policy(nn.Module):
    def __init__(self, emb=24, hid=128, dropout=0.2):
        super().__init__()
        self.card_emb = nn.Embedding(len(VOCAB), emb, padding_idx=0)
        self.ev_mlp = nn.Sequential(nn.Linear(emb + 5, hid), nn.ReLU(), nn.Linear(hid, hid))
        self.ev_attn = nn.Linear(hid, 1)
        self.num_mlp = nn.Sequential(nn.Linear(NUM_DIM, hid), nn.ReLU())
        self.trunk = nn.Sequential(nn.Linear(2 * hid, hid), nn.ReLU(), nn.Dropout(dropout), nn.Linear(hid, hid), nn.ReLU())
        self.play = nn.Linear(hid, 1)
        self.card = nn.Linear(hid, 8)
        self.pos = nn.Sequential(nn.Linear(hid + 8, hid), nn.ReLU(), nn.Dropout(dropout))
        self.x = nn.Linear(hid, 18)
        self.y = nn.Linear(hid, 32)

    def encode(self, num, ev):
        idx = ev[..., 1].long()
        feats = torch.cat([self.card_emb(idx), ev[..., [0, 2, 3, 4, 5]]], -1)
        h = self.ev_mlp(feats)
        mask = idx > 0
        a = self.ev_attn(h).squeeze(-1).masked_fill(~mask, -1e9)
        w = torch.softmax(a, -1) * mask.any(-1, keepdim=True)
        pooled = (w.unsqueeze(-1) * h).sum(1)
        return self.trunk(torch.cat([self.num_mlp(num), pooled], -1))

    def forward(self, num, ev, card=None):
        z = self.encode(num, ev)
        play = self.play(z).squeeze(-1)
        avail = num[:, AVAIL_SLICE] > 0.5
        card_logits = self.card(z).masked_fill(~avail, -1e9)
        if card is None:
            card = card_logits.argmax(-1)
        p = self.pos(torch.cat([z, F.one_hot(card.clamp(min=0), 8).float()], -1))
        return play, card_logits, self.x(p), self.y(p)
