"""离线对战：两个 AI 各控制一方，在交互式离线引擎（NativeEnv）里对打，统计胜负。

AI 接口与实战 bot 相同：decide(frame, now) -> Action(slot, card, x, y, reason) | None，
坐标是"己方在下方（y 大）"的视角；这里负责换算成原生出牌指令（卡组位置、pd 编码、引擎坐标）。

    from crai.env.match import play_match, HOG26_SPEC
    res = play_match(agent_a, agent_b, deck_a=HOG26_SPEC, deck_b=HOG26_SPEC, seed=1)
"""

from __future__ import annotations

import copy
import json
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
import royaleapi_to_native as C  # noqa: E402

from crai.data import cards  # noqa: E402
from crai.env.native_env import NativeEnv, play_cmd  # noqa: E402
from crai.reader.stream import parse_frame  # noqa: E402

DECISION_TICKS = 5
MAX_TICKS = 20 * 60 * 6


@dataclass
class DeckSpec:
    deck: list[str]                       # 8 个卡牌 slug
    forms: dict[str, int] = field(default_factory=dict)   # slug -> el（1 进化，2 英雄）
    level: int = 16
    tower: int = 159000000


HOG26_SPEC = DeckSpec(list(C.HOG26), dict(C.HOG26_FORMS))


def build_start(deck0: DeckSpec, deck1: DeckSpec, seed: int | None = None) -> dict:
    spec = {"sides": {}, "plays": [], "abilities": [], "meta": {}}
    for side, d in ((0, deck0), (1, deck1)):
        spec["sides"][side] = {"name": f"side{side}", "deck": list(d.deck), "forms": dict(d.forms),
                               "levels": {s: d.level for s in d.deck}, "tower": d.tower}
    nat, _ = C.build_from_spec(spec)
    nat["cmd"] = []
    nat["endTick"] = MAX_TICKS
    if seed is not None:
        nat["rndSeed"] = int(seed)
    return nat


def _frame_for(raw: dict, owner: int):
    f = parse_frame(json.dumps(dict(raw, t=raw["tick"] / 20.0, battle=True)))
    idx = next(i for i, p in enumerate(f.players) if p.side == owner)
    f.own_index = lambda i=idx: i          # 实例属性覆盖方法：明确己方，不依赖 own_deck.json
    return f, f.players[idx]


def _own_king_low(raw: dict, owner: int) -> bool:
    k = next((e for e in raw["entities"] if is_tower(e) and e["x"] == 9000 and e["side"] == owner), None)
    return k is not None and k["y"] < 16000


@dataclass
class MatchResult:
    crowns: dict[int, int]
    ticks: int
    plays: dict[int, int]
    rejected: dict[int, int]
    seconds: float

    @property
    def winner(self) -> int | None:
        if self.crowns[0] == self.crowns[1]:
            return None
        return 0 if self.crowns[0] > self.crowns[1] else 1


# 6 座塔的固定坐标。card == -1 不只是塔：对局中途生成的一些实体（藤蔓、炸弹等）card 也是 -1，
# 以前把它们算成"存活的塔"，出现过负皇冠（2026-11-21 评测 Everlight 卡组池局）。塔不会移动，按坐标认。
TOWER_POS = {(9000, 3000), (3500, 6500), (14500, 6500), (9000, 29000), (3500, 25500), (14500, 25500)}


def is_tower(e: dict) -> bool:
    return e["card"] == -1 and (e["x"], e["y"]) in TOWER_POS


def crowns_of(raw: dict) -> dict[int, int]:
    alive = {0: 0, 1: 0}
    king = {}
    for e in raw["entities"]:
        if is_tower(e):
            if e["x"] == 9000:
                king[e["side"]] = e["hp"]
            if e["hp"] > 0:
                alive[e["side"]] += 1
    return {s: (3 if king.get(1 - s, 1) <= 0 else 3 - alive[1 - s]) for s in (0, 1)}


class MatchState:
    """一局离线对战的逐步推进（每 5 tick 一轮）：
        ms = MatchState(a0, a1, d0, d1, seed, env); ms.start()
        while not ms.done:
            need = ms.pre()                      # 规则类 AI 直接出牌；FirstLight 类返回 [(owner, agent, 观测)]
            for owner, ag, obs in need: ms.post(owner, ag, ag.session.decide(obs, force_act=ag.force))
            ms.step()
    多局并行时（crai/env/vec_match.py）各局的 pre 收集起来批量推理，再逐局 post / step。"""

    def __init__(self, agent0, agent1, deck0: DeckSpec = HOG26_SPEC, deck1: DeckSpec = HOG26_SPEC,
                 seed: int | None = None, env: NativeEnv | None = None, verbose: bool = False):
        self.env = env
        self.start_json = build_start(deck0, deck1, seed)
        b = self.start_json["battle"]
        self.acct = {0: b["avatar0"]["accountID.lo"], 1: b["avatar1"]["accountID.lo"]}
        self.specs = {0: deck0, 1: deck1}
        self.agents = {0: agent0, 1: agent1}
        self.enc = C.PdEncoder()
        self.plays, self.rejected, self.pending = {0: 0, 1: 0}, {0: 0, 1: 0}, {}
        self.verbose = verbose
        self.raw = None
        self.t0 = time.time()

    def start(self):
        self.t0 = time.time()
        self.raw = self.env.reset(self.start_json)
        return self

    @property
    def done(self) -> bool:
        return bool(self.raw.get("ended")) or self.raw["tick"] >= MAX_TICKS

    def pre(self) -> list:
        raw, env = self.raw, self.env
        tick = raw["tick"]
        need = []
        for owner in (0, 1):
            ag = self.agents[owner]
            if hasattr(ag, "prepare"):            # FirstLight 类：每 5 tick 必须调用（模型有记忆）
                obs = ag.prepare(raw, self.acct[owner])
                if obs is not None:
                    need.append((owner, ag, obs))
                continue
            if hasattr(ag, "decide_raw"):         # 其他直接输出原生指令的 AI
                for cmd in ag.decide_raw(raw, self.acct[owner]):
                    self._inject(owner, cmd)
                continue
            f, me = _frame_for(raw, owner)
            # 上一条指令是否被执行：出牌序号没涨就算被拒（圣水不够、位置不合法等）
            if owner in self.pending and tick >= self.pending[owner][0]:
                if me.seq <= self.pending[owner][1]:
                    self.rejected[owner] += 1
                del self.pending[owner]
            if owner in self.pending:
                continue
            act = ag.decide(f, now=tick / 20.0)
            if act is None or not (0 <= act.slot < len(me.hand)):
                continue
            d = me.hand[act.slot]
            cid = me.deck[d]
            slug = cards.ID2SLUG.get(cid)
            if slug is None:
                continue
            x, y = act.x, act.y
            if _own_king_low(raw, owner):           # 视角坐标 → 引擎坐标
                x, y = 18000 - x, 32000 - y
            pd = self.enc.encode(owner, slug, d, self.specs[owner].forms.get(slug, 0))
            env.inject(play_cmd(tick, self.acct[owner], cid, pd, int(x), int(y)))
            self.plays[owner] += 1
            self.pending[owner] = (tick + 25, me.seq)
            if self.verbose:
                print(f"  t={tick:5d} side{owner} {slug} -> ({x},{y}) {act.reason}")
        return need

    def post(self, owner: int, agent, decision):
        for cmd in agent.finish(decision):
            self._inject(owner, cmd)

    def _inject(self, owner: int, cmd: dict):
        if cmd.get("ct") == 189:                  # 英雄技能（不算出牌数）
            ok = self.env.inject(cmd)
            self.abilities = getattr(self, "abilities", {0: 0, 1: 0})
            self.abilities[owner] += int(bool(ok))
            if self.verbose:
                print(f"  t={self.raw['tick']:5d} side{owner} 技能 cgid={cmd['c']['cgid']} ok={ok}")
            return
        if self.env.inject(cmd):
            self.plays[owner] += 1
            if self.verbose:
                x = cmd["c"]
                print(f"  t={self.raw['tick']:5d} side{owner} {cards.ID2SLUG.get(x['sel']['os'])} -> "
                      f"({x['px']},{x['py']}) FL")

    def step(self):
        self.raw = self.env.step(DECISION_TICKS)

    def result(self) -> MatchResult:
        return MatchResult(crowns_of(self.raw), self.raw["tick"], self.plays, self.rejected,
                           round(time.time() - self.t0, 1))


def play_match(agent0, agent1, deck0: DeckSpec = HOG26_SPEC, deck1: DeckSpec = HOG26_SPEC,
               seed: int | None = None, env: NativeEnv | None = None, verbose: bool = False) -> MatchResult:
    own_env = env is None
    env = env or NativeEnv()
    ms = MatchState(agent0, agent1, deck0, deck1, seed, env, verbose)
    try:
        ms.start()
        while not ms.done:
            for owner, ag, obs in ms.pre():
                ms.post(owner, ag, ag.session.decide(obs, force_act=ag.force))
            ms.step()
    finally:
        if own_env:
            env.close()
    return ms.result()
