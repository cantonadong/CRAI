"""策略网络 v2：以场上单位为 token 的 Transformer。输入格式见 crai/data/dataset_v2.py。

token：1 个全局 token（标量特征）+ 10 个手牌 token（己方手牌 4、下一张、对方手牌 4、下一张）+ 最多 E_MAX 个单位 token。
输出：
  type_logits [B, 3]   不出 / 出牌 / 技能
  slot_logits [B, 4]   出哪张手牌（全局 token 与己方 4 个手牌 token 的匹配分数）
  pos_logits  [B, 576] 落点格子 18×32（以所选手牌为条件；索引 = y*18 + x）
"""

from __future__ import annotations

import torch
import torch.nn as nn

from crai.data.dataset_v2 import E_FEAT, S_DIM

VOCAB_SIZE = 512       # 词表上限（dataset_v2.Vocab 动态增长，远小于此）
GRID = 18 * 32


class PolicyV2(nn.Module):
    def __init__(self, d=128, layers=4, heads=4, dropout=0.1):
        super().__init__()
        self.card = nn.Embedding(VOCAB_SIZE, d, padding_idx=0)
        self.role = nn.Embedding(10, d)                     # 手牌 token 的角色
        self.ent = nn.Linear(E_FEAT - 1, d)                 # 单位的数值特征（不含词表下标）
        self.glob = nn.Sequential(nn.Linear(S_DIM, d), nn.ReLU(), nn.Linear(d, d))
        self.type_tok = nn.Parameter(torch.zeros(3, d))     # 全局 / 手牌 / 单位
        enc = nn.TransformerEncoderLayer(d, heads, 4 * d, dropout, batch_first=True, norm_first=True)
        self.enc = nn.TransformerEncoder(enc, layers)
        self.norm = nn.LayerNorm(d)
        self.type_head = nn.Linear(d, 3)
        self.slot_q = nn.Linear(d, d)
        self.pos_head = nn.Sequential(nn.Linear(2 * d, 2 * d), nn.ReLU(), nn.Linear(2 * d, GRID))

    def encode(self, scal, cards, ents):
        B = scal.shape[0]
        g = self.glob(scal).unsqueeze(1) + self.type_tok[0]
        roles = torch.arange(10, device=cards.device).expand(B, 10)
        h = self.card(cards.long().clamp(0, VOCAB_SIZE - 1)) + self.role(roles) + self.type_tok[1]
        eid = ents[..., 0].long().clamp(0, VOCAB_SIZE - 1)
        e = self.card(eid) + self.ent(ents[..., 1:]) + self.type_tok[2]
        x = torch.cat([g, h, e], 1)
        pad = torch.cat([torch.zeros(B, 11, dtype=torch.bool, device=x.device), ents[..., -1] < 0.5], 1)
        z = self.norm(self.enc(x, src_key_padding_mask=pad))
        return z[:, 0], z[:, 1:5]                          # 全局、己方 4 张手牌

    def forward(self, scal, cards, ents, slot=None):
        zg, zh = self.encode(scal, cards, ents)
        type_logits = self.type_head(zg)
        slot_logits = torch.einsum("bd,bkd->bk", self.slot_q(zg), zh) / zg.shape[-1] ** 0.5
        if slot is None:
            slot = slot_logits.argmax(-1)
        chosen = zh[torch.arange(zh.shape[0], device=zh.device), slot.clamp(0, 3)]
        pos_logits = self.pos_head(torch.cat([zg, chosen], -1))
        return type_logits, slot_logits, pos_logits
