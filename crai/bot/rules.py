"""规则 bot（第一阶段基线）：野猪 2.6 卡组。

坐标系：统一换算到"己方在下方"的视角（own frame）：X 0..18000 左→右，Y 0..32000 上→下，
己方半场 Y > 16000，河道约 Y 15000–17000，桥在 X≈3500 / 14500。
决策输出 (手牌槽位, X, Y)，由 crai.client.control.deploy 执行。
"""

from __future__ import annotations

import math
import time
from dataclasses import dataclass

from crai.reader.stream import Entity, Frame

SKELETONS, MUSKETEER, CANNON, HOG = 26000010, 26000014, 27000000, 26000021
FIREBALL, LOG, ICE_SPIRIT, ICE_GOLEM = 28000000, 28000011, 26000030, 26000038
COST = {SKELETONS: 1, MUSKETEER: 4, CANNON: 3, HOG: 4, FIREBALL: 4, LOG: 2, ICE_SPIRIT: 1, ICE_GOLEM: 2}
NAME = {SKELETONS: "骷髅兵", MUSKETEER: "火枪手", CANNON: "加农炮", HOG: "野猪", FIREBALL: "火球",
        LOG: "圆木", ICE_SPIRIT: "冰精灵", ICE_GOLEM: "冰人"}

KIND_TOWER, KIND_TROOP = 13, 15  # 观测：12=未激活的国王塔 13=激活的塔 14=部署中 15=部队


def is_tower(e: Entity) -> bool:
    """塔没有卡牌 ID（card == -1）；不能只看 kind（国王塔未激活时 kind=12）。"""
    return e.card == -1 and e.kind in (12, 13)


def is_king(e: Entity) -> bool:
    return is_tower(e) and e.x == 9000
LEFT, RIGHT = 0, 1
BRIDGE_X = {LEFT: 3500, RIGHT: 14500}


@dataclass
class Action:
    slot: int
    card: int
    x: int
    y: int
    reason: str


@dataclass
class View:
    """own frame 下的战局视图。"""
    elixir: float
    hand: list[int]
    own_troops: list[Entity]
    enemy_troops: list[Entity]
    enemy_towers: list[Entity]
    own_towers: list[Entity]


def to_own_frame(frame: Frame) -> View:
    me = frame.players[frame.own_index()]
    kings = [e for e in frame.entities if is_king(e) and e.side == me.side]
    flip = bool(kings) and kings[0].y < 16000  # 己方国王塔在内存上方 -> 翻转到下方

    def conv(e: Entity) -> Entity:
        if not flip:
            return e
        return Entity(e.id, e.kind, e.side, 18000 - e.x, 32000 - e.y, e.card, e.lvl, e.hp, e.max_hp)

    ents = [conv(e) for e in frame.entities if e.hp != 0]
    own = [e for e in ents if e.side == me.side]
    enemy = [e for e in ents if e.side != me.side]
    return View(
        elixir=me.elixir,
        hand=me.hand_cards(),
        own_troops=[e for e in own if not is_tower(e)],
        enemy_troops=[e for e in enemy if not is_tower(e)],
        enemy_towers=[e for e in enemy if is_tower(e)],
        own_towers=[e for e in own if is_tower(e)],
    )


def lane_of(x: float) -> int:
    return LEFT if x < 9000 else RIGHT


def dist(a: Entity, x: float, y: float) -> float:
    return math.hypot(a.x - x, a.y - y)


def is_troop_card(card: int) -> bool:
    return 26000000 <= card < 27000000 or card == -1


class Hog26Bot:
    def __init__(self):
        self.last_play = 0.0
        self.last_hog = 0.0
        self.pending_seq: int | None = None  # 下牌后等待出牌序号超过该值，避免重复下同一张牌
        self.failed_plays = 0
        self.need_deselect = False

    # ---------- 工具 ----------
    def _slot(self, v: View, card: int) -> int | None:
        if card in v.hand and v.elixir >= COST[card]:
            return v.hand.index(card)
        return None

    def _first(self, v: View, cards: list[int]) -> tuple[int, int] | None:
        for c in cards:
            s = self._slot(v, c)
            if s is not None:
                return s, c
        return None

    def _attack_lane(self, v: View) -> int:
        """打对方血量更低的公主塔所在路；某路塔已拆则打另一路（野猪会直奔国王塔，同样有效）。"""
        princess = {lane_of(t.x): t for t in v.enemy_towers if t.x != 9000}
        if len(princess) < 2:
            alive = list(princess)
            return 1 - alive[0] if alive else LEFT
        l, r = princess[LEFT], princess[RIGHT]
        return LEFT if l.hp <= r.hp else RIGHT

    # ---------- 决策 ----------
    def decide(self, frame: Frame, now: float | None = None) -> Action | None:
        now = time.time() if now is None else now
        seq = frame.players[frame.own_index()].seq
        if self.pending_seq is not None:
            if seq > self.pending_seq:
                self.pending_seq = None
            elif now - self.last_play > 1.5:
                self.pending_seq = None
                self.failed_plays += 1
                self.need_deselect = True  # 点击下牌失败时卡会保持选中，执行方应先取消选中
            else:
                return None
        if now - self.last_play < 0.5:
            return None
        v = to_own_frame(frame)
        act = self._defend(v) or self._attack(v, now) or self._avoid_leak(v)
        if act:
            self.last_play = now
            self.pending_seq = seq
            if act.card == HOG:
                self.last_hog = now
        return act

    def _defend(self, v: View) -> Action | None:
        threats = [e for e in v.enemy_troops if e.y > 14500 and is_troop_card(e.card)]  # 已到河边或过河的部队
        if not threats:
            return None
        # 最靠近己方塔的威胁
        t = max(threats, key=lambda e: e.y)
        lane = lane_of(t.x)
        # 一群小兵扎堆 -> 圆木/火球
        cluster = [e for e in v.enemy_troops if is_troop_card(e.card) and dist(e, t.x, t.y) < 2500]
        if len(cluster) >= 3:
            cx = sum(e.x for e in cluster) / len(cluster)
            cy = sum(e.y for e in cluster) / len(cluster)
            pick = self._first(v, [LOG, FIREBALL])
            if pick:
                s, c = pick
                if c == LOG:  # 圆木从目标前方 2 格往敌方滚
                    return Action(s, c, int(cx), int(min(cy + 2000, 31500)), f"圆木清群({len(cluster)})")
                return Action(s, c, int(cx), int(cy), f"火球清群({len(cluster)})")
        # 单体威胁：加农炮放中间拉扯，否则冰人/火枪手/骷髅兵近身
        if t.y > 15500 and not any(e.card == CANNON for e in v.own_troops):
            s = self._slot(v, CANNON)
            if s is not None:
                return Action(s, CANNON, 9000 if lane == RIGHT else 8000, 21500, f"加农炮防守 card={t.card}")
        if t.y > 17500:
            pick = self._first(v, [ICE_GOLEM, MUSKETEER, SKELETONS, ICE_SPIRIT])
            if pick:
                s, c = pick
                if c == MUSKETEER:  # 火枪手放在后方输出
                    return Action(s, c, BRIDGE_X[lane] + (1500 if lane == LEFT else -1500), 26500, "火枪手防守")
                y = min(t.y + 2000, 30500)
                return Action(s, c, int(t.x), int(y), f"{NAME[c]}拦截 card={t.card}")
        return None

    def _attack(self, v: View, now: float) -> Action | None:
        if now - self.last_hog < 6.0:
            return None
        if v.elixir < 7.0 and not (v.elixir >= 4 and self._enemy_low_elixir_hint(v)):
            return None
        s = self._slot(v, HOG)
        if s is None:
            return None
        lane = self._attack_lane(v)
        return Action(s, HOG, BRIDGE_X[lane], 17500, f"野猪进攻 {'左' if lane == LEFT else '右'}路")

    def _enemy_low_elixir_hint(self, v: View) -> bool:
        # 对方刚在我方半场投入了大量兵力（已在防守），反打
        return len([e for e in v.enemy_troops if e.y > 16000 and is_troop_card(e.card)]) >= 2

    def _avoid_leak(self, v: View) -> Action | None:
        if v.elixir < 9.3:
            return None
        # 圣水快满：后场慢放一张便宜卡循环（优先冰人在国王塔前，火枪手在公主塔后）
        lane = self._attack_lane(v)
        if (p := self._first(v, [ICE_GOLEM, ICE_SPIRIT, SKELETONS])):
            s, c = p
            return Action(s, c, BRIDGE_X[lane], 28500 if c != ICE_GOLEM else 24000, "防溢出循环")
        if (s := self._slot(v, MUSKETEER)) is not None:
            return Action(s, MUSKETEER, BRIDGE_X[lane] + (1500 if lane == LEFT else -1500), 28500, "防溢出火枪手")
        if (s := self._slot(v, CANNON)) is not None and not any(e.card == CANNON for e in v.own_troops):
            return Action(s, CANNON, 9000, 22500, "防溢出加农炮")
        return None
