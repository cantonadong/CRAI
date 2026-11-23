"""v2 bot：用 PolicyV2（crai/model/policy_v2.py）在真实对局中决策，特征与 crai/data/dataset_v2.py 完全一致。

- 每 STEP tick（0.25s）决策一次：P(出牌) ≥ 阈值时，取出哪张手牌和落点格子。
- 坐标：模型在"己方在下方（y 大）"的视角里工作；己方国王塔在 y 小的一侧时，特征和输出都旋转 180°。
- 安全规则：圣水不够不出；非法术卡的落点限制在己方半场（y ≥ 17000，视角坐标）；出牌后 1.5 秒内序号没变算失败。
- 技能（类型 2）暂不执行。
"""

from __future__ import annotations

import json
import math
import time
from pathlib import Path

import numpy as np
import torch

from crai.bot.rules import Action
from crai.data import cards
from crai.data.dataset_v2 import DOUBLE_TICK, E_FEAT, E_MAX, OT_TICK, STEP
from crai.model.policy_v2 import PolicyV2
from crai.reader.stream import Frame

ROOT = Path(__file__).resolve().parents[2]
TICKS_PER_SEC = 20


class V2Bot:
    # 阈值：优先用训练时在验证集上校准、存在模型文件里的值（预测出牌比例 = 真人出牌比例）；旧模型没有时用 0.12
    def __init__(self, model_path: Path = ROOT / "models" / "hog26_v2.pt", threshold: float | None = None):
        ck = torch.load(model_path, map_location="cpu")
        if threshold is None:
            threshold = float(ck.get("threshold", 0.12))
        self.vocab = {int(k): v for k, v in ck["vocab"].items()}
        self.model = PolicyV2()
        self.model.load_state_dict(ck["state"])
        self.model.eval()
        self.thr = threshold
        self.t0: float | None = None
        self.last_decide_tick = -STEP
        self.last_play = 0.0
        self.pending_seq: int | None = None
        self.failed_plays = 0
        self.need_deselect = False
        # 决策日志：每次决策的出牌概率和输入特征，用来对比实战与训练数据的分布
        log_dir = ROOT / "data" / "bot_runs"
        log_dir.mkdir(parents=True, exist_ok=True)
        self.log = open(log_dir / f"v2_decisions_{time.strftime('%Y%m%d_%H%M%S')}.jsonl", "w", encoding="utf-8")

    def _v(self, cid: int) -> int:
        return 1 if cid is None or cid < 0 else self.vocab.get(cid, 1)

    def features(self, f: Frame, tick: int):
        me_i = f.own_index()
        me, op = f.players[me_i], f.players[1 - me_i]
        own = me.side
        king = next((e for e in f.entities if e.card == -1 and e.x == 9000 and e.side == own), None)
        flip = king is not None and king.y < 16000

        def rot(x, y):
            return (18000 - x, 32000 - y) if flip else (x, y)

        def hand_ids(p):
            h = [p.deck[i] if 0 <= i < 8 else -1 for i in p.hand]
            n = p.deck[p.next[0]] if p.next and 0 <= p.next[0] < 8 else -1
            return h, n

        mh, mn = hand_ids(me)
        oh, on = hand_ids(op)
        cards_ = np.array([self._v(c) for c in mh + [mn] + oh + [on]], np.int64)
        towers = np.zeros(6, np.float32)
        for e in f.entities:
            if e.card != -1 or e.max_hp <= 0:
                continue
            x, _ = rot(e.x, e.y)
            k = 2 if e.x == 9000 else (0 if x < 9000 else 1)
            towers[(0 if e.side == own else 3) + k] = max(0, e.hp) / e.max_hp
        phase = [tick < DOUBLE_TICK, DOUBLE_TICK <= tick < OT_TICK, tick >= OT_TICK]
        scal = np.concatenate([[tick / OT_TICK], np.array(phase, np.float32), [me.elixir / 10, op.elixir / 10],
                               towers, [me.seq / 50, op.seq / 50]]).astype(np.float32)
        ents = np.zeros((E_MAX, E_FEAT), np.float32)
        units = [e for e in f.entities if e.card != -1]
        units.sort(key=lambda e: e.side != own)
        for i, e in enumerate(units[:E_MAX]):
            x, y = rot(e.x, e.y)
            mx = max(e.max_hp, 1)
            ents[i] = [self._v(e.raw_card), float(e.side == own), x / 18000, y / 32000,
                       max(e.hp, 0) / mx if e.max_hp > 0 else 1.0, math.log1p(max(e.max_hp, 0)) / 10, e.kind / 15, 1.0]
        t = lambda a: torch.tensor(a).unsqueeze(0)  # noqa: E731
        return t(scal), t(cards_), t(ents), me, mh

    def decide(self, f: Frame, now: float | None = None) -> Action | None:
        now = time.time() if now is None else now
        if self.t0 is None:
            self.t0 = now
        tick = int((now - self.t0) * TICKS_PER_SEC)
        me = f.players[f.own_index()]
        if self.pending_seq is not None:
            if me.seq > self.pending_seq:
                self.pending_seq = None
            elif now - self.last_play > 1.5:
                self.pending_seq = None
                self.failed_plays += 1
                self.need_deselect = True
            else:
                return None
        if tick - self.last_decide_tick < STEP:
            return None
        self.last_decide_tick = tick
        s, c, e, me, mh = self.features(f, tick)
        with torch.no_grad():
            tl, sl, _ = self.model(s, c, e)
            p = torch.softmax(tl, -1)[0]
        self.log.write(json.dumps({"tick": tick, "p": [round(v, 4) for v in p.tolist()],
                                   "scal": [round(float(v), 4) for v in s[0]], "cards": c[0].tolist(),
                                   "n_ents": int((e[0, :, -1] > 0.5).sum()), "ents": e[0][e[0, :, -1] > 0.5].tolist()})
                       + "\n")
        self.log.flush()
        leaking = me.elixir >= 9.7         # 圣水快满：强制出牌，避免浪费
        if p[1].item() < self.thr and not leaking:
            return None
        for slot in sl[0].argsort(descending=True).tolist():
            cid = mh[slot]
            if cid < 0:
                continue
            if me.elixir < cards.cost(cid):
                return None            # 首选牌圣水不够：等（和人类一样）
            with torch.no_grad():
                _, _, pl = self.model(s, c, e, slot=torch.tensor([slot]))
            g = int(pl[0].argmax())
            gx, gy = g % 18, g // 18
            x, y = gx * 1000 + 500, gy * 1000 + 500
            if not (28000000 <= cid < 29000000) and y < 17500:
                y = 17500              # 非法术卡：限制在己方半场
            self.last_play = now
            self.pending_seq = me.seq
            return Action(slot, cid, x, y, f"v2 p={p[1].item():.2f}{' 满圣水' if leaking else ''}")
        return None
