"""混合 bot：模仿学习模型决定"何时出、出哪张"，落点由规则根据实时战场计算。

实战中重建与训练相同的特征（crai.data.dataset.features_at）：
  - 出牌事件：双方出牌序号增加时记录 last_card；己方落点即我们下牌的目标点；对方落点取该卡新出现实体的位置，法术记为位置未知
  - 圣水/手牌/下一张：直接用内存精确值；时间：开局起 wall clock × 20 tick/s
"""

from __future__ import annotations

import math
import time
from pathlib import Path

import numpy as np
import torch

from crai.bot.rules import (BRIDGE_X, CANNON, FIREBALL, HOG, ICE_GOLEM, ICE_SPIRIT, LEFT, LOG, MUSKETEER, RIGHT,
                            SKELETONS, Action, is_troop_card, lane_of, to_own_frame)
from crai.data import cards
from crai.data.dataset import TICKS_PER_SEC, Event, features_at
from crai.model.policy import Policy
from crai.reader.stream import Frame

ROOT = Path(__file__).resolve().parents[2]


class LiveEvents:
    """从连续帧中重建双方出牌事件（己方在下方的坐标系）。"""

    def __init__(self):
        self.events: list[Event] = []
        self.last_seq: dict[int, int] = {}
        self.pending_enemy: list[tuple[int, str, float]] = []  # (事件下标, slug, 截止时间)
        self.seen_ids: set[str] = set()
        self.own_targets: list[tuple[int, int, int]] = []  # 我们最近下牌的 (card_id, x, y)

    def note_own_target(self, card_id: int, x: int, y: int):
        self.own_targets.append((card_id, x, y))

    def update(self, frame: Frame, tick: int, now: float):
        me_i = frame.own_index()
        v = to_own_frame(frame)
        new_ents = [e for e in v.enemy_troops if e.id not in self.seen_ids]
        for e in frame.entities:
            self.seen_ids.add(e.id)
        for i, p in enumerate(frame.players):
            prev = self.last_seq.get(i)
            self.last_seq[i] = p.seq
            if prev is None or p.seq <= prev:
                continue
            slug = cards.ID2SLUG.get(p.last_card, "<unk>")
            own = i == me_i
            if own:
                tgt = next((t for t in reversed(self.own_targets) if t[0] == p.last_card), None)
                x, y = (tgt[1], tgt[2]) if tgt else (None, None)
                self.events.append(Event(tick, True, slug, x, y))
            else:
                self.events.append(Event(tick, False, slug, None, None))
                self.pending_enemy.append((len(self.events) - 1, slug, now + 2.5))
        # 为对方事件补落点：该卡新出现的实体
        still = []
        for idx, slug, deadline in self.pending_enemy:
            cid = cards.card_id(slug)
            hit = next((e for e in new_ents if e.card == cid), None)
            if hit is not None:
                ev = self.events[idx]
                self.events[idx] = Event(ev.tick, False, slug, hit.x, hit.y)
            elif now < deadline:
                still.append((idx, slug, deadline))
        self.pending_enemy = still


class ILBot:
    def __init__(self, model_path: Path = ROOT / "models" / "hog26_il.pt", play_threshold: float = 0.45):
        ck = torch.load(model_path, map_location="cpu")
        self.deck_slugs: list[str] = ck["deck"]
        self.deck_ids = [cards.card_id(s) for s in self.deck_slugs]
        self.model = Policy()
        self.model.load_state_dict(ck["state"])
        self.model.eval()
        self.thr = play_threshold
        self.events = LiveEvents()
        self.t0: float | None = None
        self.last_play = 0.0
        self.pending_seq: int | None = None
        self.failed_plays = 0
        self.need_deselect = False
        self.last_idle = time.time()

    # ---------- 特征 ----------
    def _features(self, frame: Frame, tick: int):
        me = frame.players[frame.own_index()]
        opp = frame.players[1 - frame.own_index()]
        num, ev = features_at(tick, self.events.events, self.deck_slugs, me.elixir, opp.elixir)
        hand = set(me.hand_cards())
        num[6:14] = [1.0 if cid in hand else 0.0 for cid in self.deck_ids]   # 精确手牌
        nxt = np.zeros(8, np.float32)
        if me.next_card() in self.deck_ids:
            nxt[self.deck_ids.index(me.next_card())] = 1.0
        num[14:22] = nxt
        num[22] = 1.0
        return torch.tensor(num).unsqueeze(0), torch.tensor(ev).unsqueeze(0)

    # ---------- 决策 ----------
    def decide(self, frame: Frame, now: float | None = None) -> Action | None:
        now = time.time() if now is None else now
        if self.t0 is None:
            self.t0 = now
        tick = int((now - self.t0) * TICKS_PER_SEC)
        self.events.update(frame, tick, now)
        me = frame.players[frame.own_index()]
        if self.pending_seq is not None:
            if me.seq > self.pending_seq:
                self.pending_seq = None
            elif now - self.last_play > 1.5:
                self.pending_seq = None
                self.failed_plays += 1
                self.need_deselect = True
            else:
                return None
        if now - self.last_play < 0.5:
            return None
        num, ev = self._features(frame, tick)
        with torch.no_grad():
            play_l, card_l, x_l, y_l = self.model(num, ev)
        p = torch.sigmoid(play_l).item()
        v = to_own_frame(frame)
        threat = self._threat(v)
        leaking = me.elixir >= 9.8
        if p < self.thr and not leaking and not (threat and now - self.last_play > 2.0 and me.elixir >= 3):
            return None
        hand = me.hand_cards()
        order = card_l[0].argsort(descending=True).tolist()
        for slot8 in order:
            cid = self.deck_ids[slot8]
            if cid not in hand:
                continue
            if me.elixir < cards.cost(cid):
                return None  # 首选牌圣水不够：等待（与人类行为一致）
            with torch.no_grad():
                _, _, xl, yl = self.model(num, ev, card=torch.tensor([slot8]))
            mx, my = int(xl.argmax()) * 1000 + 500, int(yl.argmax()) * 1000 + 500
            pos = self._place(cid, v, mx, my, threat)
            if pos is None:
                continue
            act = Action(hand.index(cid), cid, pos[0], pos[1], f"IL p={p:.2f} {cards.ID2SLUG.get(cid, cid)}")
            self.last_play = now
            self.pending_seq = me.seq
            self.events.note_own_target(cid, pos[0], pos[1])
            return act
        return None

    # ---------- 落点规则 ----------
    @staticmethod
    def _threat(v):
        ts = [e for e in v.enemy_troops if e.y > 14500 and is_troop_card(e.card)]
        return max(ts, key=lambda e: e.y) if ts else None

    def _place(self, cid, v, mx, my, threat):
        own_half = lambda x, y: (int(min(max(x, 500), 17500)), int(min(max(y, 17500), 30500)))  # noqa: E731
        lane = lane_of(mx)
        if cid == HOG:
            return BRIDGE_X[lane], 17500
        if cid in (FIREBALL, LOG):
            enemies = [e for e in v.enemy_troops if is_troop_card(e.card)]
            if enemies:
                # 选周围敌方单位最多的位置
                best = max(enemies, key=lambda e: sum(math.hypot(e.x - o.x, e.y - o.y) < 2500 for o in enemies))
                if cid == LOG:
                    return int(best.x), int(min(best.y + 2000, 31500))
                return int(best.x), int(best.y)
            if cid == FIREBALL:  # 无部队：打血量较低的公主塔
                towers = [t for t in v.enemy_towers if t.x != 9000 and t.hp > 0]
                if towers:
                    t = min(towers, key=lambda t: t.hp)
                    return int(t.x), int(t.y)
                return None
            return BRIDGE_X[lane], 17500  # 圆木无目标：跟随进攻路线
        if threat is not None:
            if cid == CANNON:
                return (9000 if lane_of(threat.x) == RIGHT else 8000), 21500
            if cid == MUSKETEER:
                l = lane_of(threat.x)
                return BRIDGE_X[l] + (1500 if l == LEFT else -1500), 26500
            return own_half(threat.x, threat.y + 2000)
        return own_half(mx, my)
