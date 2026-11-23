"""从 RoyaleAPI 回放构建模仿学习数据集（视角：指定卡组的玩家，坐标统一为"己方在下方"）。

决策点：每 STEP_TICKS 个 tick（0.5s）一个；若该时间窗内该玩家出了牌，标签为 (出牌, 卡组槽位, x 格, y 格)，否则为"不出"。
特征只使用回放与实战读取器都能得到的信息，见 FEATURE 文档：
  num:   [时间, 阶段 one-hot(3), 己方圣水估计, 对方圣水估计, 手牌可用掩码(8), 下一张 one-hot(8), 手牌是否已确定,
          对方已暴露卡牌数, 最近 10s 对方在我方半场左右路出牌数(2)]
  ev:    最近 K 个出牌事件 [是否己方, 卡牌词表下标, x/18000, y/32000, 距今秒数/10, 位置是否已知]
"""

from __future__ import annotations

import glob
import json
from dataclasses import dataclass
from pathlib import Path

import numpy as np

from crai.data import cards
from crai.data.replay import Replay, load_replay

TICKS_PER_SEC = 20
STEP_TICKS = 10
K_EVENTS = 10
DOUBLE_TICK = 120 * TICKS_PER_SEC
OT_TICK = 180 * TICKS_PER_SEC
UNKNOWN_COST = 4

# 词表：0 = padding，1 = 未知卡，其余按卡牌表顺序
VOCAB = ["<pad>", "<unk>"] + sorted(cards.CARDS)
VOCAB_IDX = {s: i for i, s in enumerate(VOCAB)}


def vocab_index(slug: str) -> int:
    return VOCAB_IDX.get(slug, 1)


def regen_ticks(tick: int) -> float:
    """每点圣水需要的 tick 数。"""
    if tick >= OT_TICK:
        return 2.8 * TICKS_PER_SEC / 3
    if tick >= DOUBLE_TICK:
        return 2.8 * TICKS_PER_SEC / 2
    return 2.8 * TICKS_PER_SEC


@dataclass
class Event:
    tick: int
    own: bool
    slug: str
    x: int | None
    y: int | None


class ElixirTracker:
    def __init__(self):
        self.e, self.tick = 5.0, 0

    def advance(self, tick: int):
        while self.tick < tick:
            self.e = min(10.0, self.e + 1.0 / regen_ticks(self.tick))
            self.tick += 1

    def spend(self, c: int):
        self.e = max(0.0, self.e - c)


def is_spell(slug: str) -> bool:
    cid = cards.card_id(slug)
    return 28000000 <= cid < 29000000


def side_at_bottom(r: Replay, side: str) -> bool:
    """回放中各方所在半场不固定：用该方部队/建筑（非法术）落点的多数判断是否在下方（y>16000）。"""
    ys = [p.y for p in r.side_plays(side, include_abilities=False) if p.y is not None and not is_spell(p.card)]
    if not ys:
        other = "opponent" if side == "team" else "team"
        ys_o = [p.y for p in r.side_plays(other, include_abilities=False) if p.y is not None and not is_spell(p.card)]
        return not (sum(y > 16000 for y in ys_o) * 2 > len(ys_o)) if ys_o else True
    return sum(y > 16000 for y in ys) * 2 > len(ys)


def perspective_events(r: Replay, side: str) -> list[Event]:
    """side: 'team'/'opponent'。转换为该玩家视角：己方在下方（必要时 180° 翻转坐标）。"""
    flip = not side_at_bottom(r, side)
    out = []
    for p in r.plays:
        if p.is_ability:
            continue
        x, y = p.x, p.y
        if flip and x is not None:
            x, y = 18000 - x, 32000 - y
        out.append(Event(p.tick, p.side == side, p.card, x, y))
    return out


NUM_DIM = 1 + 3 + 2 + 8 + 8 + 1 + 1 + 2


def features_at(tick: int, events: list[Event], deck: list[str], own_el: float, opp_el: float):
    """在 tick 时刻（只使用 tick 之前的事件）计算特征。events 需按 tick 排序。"""
    past = [e for e in events if e.tick < tick]
    own_played = [e.slug for e in past if e.own]
    last4 = own_played[-4:]
    known = len(own_played) >= 4
    avail = np.array([0.0 if s in last4 else 1.0 for s in deck])
    nxt = np.zeros(8)
    if known:
        nxt[deck.index(last4[0])] = 1.0
    phase = [tick < DOUBLE_TICK, DOUBLE_TICK <= tick < OT_TICK, tick >= OT_TICK]
    opp_seen = len({e.slug for e in past if not e.own})
    recent = [e for e in past if not e.own and tick - e.tick <= 10 * TICKS_PER_SEC and e.y is not None and e.y > 15000]
    lanes = [sum(e.x < 9000 for e in recent), sum(e.x >= 9000 for e in recent)]
    num = np.concatenate([[tick / OT_TICK], np.array(phase, float), [own_el / 10, opp_el / 10],
                          avail, nxt, [float(known)], [opp_seen / 8], np.array(lanes, float) / 3])
    ev = np.zeros((K_EVENTS, 6), np.float32)
    for i, e in enumerate(reversed(past[-K_EVENTS:])):
        has = e.x is not None
        ev[i] = [float(e.own), vocab_index(e.slug), (e.x or 0) / 18000, (e.y or 0) / 32000,
                 (tick - e.tick) / TICKS_PER_SEC / 10, float(has)]
    return num.astype(np.float32), ev


def build_game(r: Replay, side: str, deck: list[str], weight: float, mirror: bool = False):
    events = perspective_events(r, side)
    if mirror:  # 左右镜像增强：x -> 18000 - x（左右路对称）
        events = [Event(e.tick, e.own, e.slug, None if e.x is None else 18000 - e.x, e.y) for e in events]
    end = max(e.tick for e in events) + STEP_TICKS
    own_el, opp_el = ElixirTracker(), ElixirTracker()
    ei = 0
    rows = []
    for t0 in range(0, end, STEP_TICKS):
        t1 = t0 + STEP_TICKS
        own_el.advance(t0); opp_el.advance(t0)
        num, ev = features_at(t0, events, deck, own_el.e, opp_el.e)
        window = [e for e in events if t0 <= e.tick < t1]
        mine = [e for e in window if e.own]
        if mine:
            e = mine[0]
            slot = deck.index(e.slug) if e.slug in deck else -1
            if slot < 0 or e.x is None:
                label = (0, -1, -1, -1)
            else:
                label = (1, slot, min(17, max(0, e.x) // 1000), min(31, e.y // 1000))
        else:
            label = (0, -1, -1, -1)
        rows.append((num, ev, label, weight))
        # 推进到窗口末尾并结算本窗口内的出牌费用
        for e in sorted(window, key=lambda e: e.tick):
            own_el.advance(e.tick); opp_el.advance(e.tick)
            c = cards.cost(e.slug) or UNKNOWN_COST
            (own_el if e.own else opp_el).spend(c)
    return rows


def build(replay_dir: str | Path, deck: list[str], out: str | Path, win_weight: float = 1.0, augment: bool = True):
    """win_weight: 赢局样本权重（输局为 1.0）。高手输局的操作同样有学习价值，默认一视同仁。"""
    replay_dir = Path(replay_dir)
    manifest = {}
    mpath = replay_dir / "_manifest.json"
    if mpath.exists():
        for tag, info in json.loads(mpath.read_text(encoding="utf-8")).items():
            manifest[info["file"]] = info
    all_rows, game_ids = [], []
    used = 0
    for gi, p in enumerate(sorted(glob.glob(str(replay_dir / "replay*.json")))):
        r = load_replay(p)
        name = Path(p).name
        for side, st in (("team", r.team), ("opponent", r.opponent)):
            if set(st.card_counts) != set(deck):
                continue
            info = manifest.get(name, {})
            w = 1.0
            if info:
                tc, oc = int(info["team_crowns"]), int(info["opponent_crowns"])
                won = tc > oc if side == "team" else oc > tc
                w = win_weight if won else 1.0
            for mirror in ((False, True) if augment else (False,)):
                rows = build_game(r, side, deck, w, mirror)
                all_rows += rows
                game_ids += [gi] * len(rows)   # 镜像副本与原局同组，保证训练/验证按对局划分不泄漏
            used += 1
    num = np.stack([r[0] for r in all_rows])
    ev = np.stack([r[1] for r in all_rows])
    lab = np.array([r[2] for r in all_rows], np.int64)
    w = np.array([r[3] for r in all_rows], np.float32)
    np.savez_compressed(out, num=num, ev=ev, label=lab, weight=w, game=np.array(game_ids), deck=np.array(deck))
    print(f"对局 {used}，决策点 {len(all_rows)}，其中出牌 {int((lab[:, 0] == 1).sum())} -> {out}")


HOG26 = ["hog-rider", "musketeer", "cannon", "ice-golem", "ice-spirit", "skeletons", "fireball", "the-log"]

if __name__ == "__main__":
    import sys
    ROOT = Path(__file__).resolve().parents[2]
    build(sys.argv[1] if len(sys.argv) > 1 else ROOT / "replay", HOG26, ROOT / "data" / "hog26_il.npz")
