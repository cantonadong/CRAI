"""把 RoyaleAPI 回放转成 16.635.1 游戏原生回放 JSON，交给离线引擎（scripts/headless_resim.py）重演。

模板：re/native_replay/ladder_last.json（游戏自己生成的回放，经 ReplayJsonSetter 钩子抓取）。
要点（实测，见 docs/re_notes.md "离线引擎"）：
  - 出牌指令 ct=153，c.t2 = 生效 tick = RoyaleAPI data-t，c.t = t2 - 20。
  - sel.pd 引擎会校验：(费用<<28) | ((卡组位置+1)<<22) | 0x3C00（无进化时）。
  - 起手牌由 rndSeed 洗牌决定，是固定的位置排列，与卡组内容无关；模板种子下的排列见 SEED_PERM。
    用 legal_order 推出合法的起手牌和队列，再按排列把卡放进对应位置。
  - 野猪 2.6 一方固定放在 side 1（deck1/avatar1，y 大的一侧），必要时坐标旋转 180°。
用法：python scripts/royaleapi_to_native.py <royaleapi回放> [输出.json]
"""
import copy
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.data import cards  # noqa: E402
from crai.data.dataset import HOG26, side_at_bottom  # noqa: E402
from crai.data.replay import load_replay  # noqa: E402
from inject_replay import FILLERS, legal_order  # noqa: E402

TEMPLATE = ROOT / "re/native_replay/ladder_last.json"
SEED = 1795206595
# 该种子下各方起手牌（4 张）和队列（4 张，按顺序）所在的卡组位置
SEED_PERM = {0: ([5, 0, 2, 3], [6, 7, 1, 4]), 1: ([2, 1, 0, 6], [4, 7, 3, 5])}
PLAY_CT = 153
COMMAND_AGE = 20
# 野猪 2.6 惯用形态（与用户卡组、FirstLight 野猪专精模型一致）：el 1 = 进化，2 = 英雄
HOG26_FORMS = {"skeletons": 1, "cannon": 1, "musketeer": 2}
_SPECS = ROOT / "refs/FirstLight_CR/native_runner/data/competitive/card_specs.json.gz"


def evo_cycles(card_id: int) -> int:
    """进化所需循环次数（FirstLight 卡牌数据的 cycle_required；缺省 2）。"""
    if not hasattr(evo_cycles, "table"):
        import gzip
        t = {}
        if _SPECS.exists():
            for c in json.load(gzip.open(_SPECS, "rt", encoding="utf-8"))["specs"]:
                ev = c.get("evolution") or {}
                if c.get("card_id") and ev.get("cycle_required"):
                    t[c["card_id"]] = int(ev["cycle_required"])
        evo_cycles.table = t
    return evo_cycles.table.get(card_id, 2)


class PdEncoder:
    """按 16.635.1 实测规律生成 sel.pd（见 docs/re_notes.md）：
    费用<<28 | (位置+1)<<22 | 形态位<<14 | 0x3C00 | 进化计数<<7 | 标志
    形态位：0x20 可进化，0x40 英雄。进化卡第 1..N 次为普通（计数 1..N），第 N+1 次为进化版（计数 N+1，标志 1），然后归零。
    英雄卡固定：形态位 0x40，标志 2。"""

    def __init__(self):
        self.counter = {}

    def encode(self, side: int, slug: str, pos: int, form: int) -> int:
        v = (cards.cost(slug) << 28) | ((pos + 1) << 22) | 0x3C00
        if form == 1:
            cid = cards.card_id(slug)
            n = evo_cycles(cid)
            c = self.counter.get((side, cid), 0) + 1
            if c > n:
                v |= (0x20 << 14) | ((n + 1) << 7) | 1
                c = 0
            else:
                v |= (0x20 << 14) | (c << 7)
            self.counter[(side, cid)] = c
        elif form == 2:
            v |= (0x40 << 14) | 2
        return v


ABILITY_CT = 189
# 有主动技能的冠军卡（FirstLight NATIVE_CHAMPION_ABILITY_BINDINGS）；英雄形态单位的卡牌编号为 203xxxxxx
CHAMPION_IDS = {26000065, 26000069, 26000072, 26000074, 26000077, 26000093, 26000099, 26000103}


def is_ability_unit(card: int) -> bool:
    return 203000000 <= card < 204000000 or card in CHAMPION_IDS


def best_order(deck8, plays):
    """先找完全合法的 (起手, 队列)；没有时取能让最长出牌前缀合法的那种。返回 (hand, queue, 合法前缀长度)。"""
    o = legal_order(deck8, plays)
    if o is not None:
        return o[0], o[1], len(plays)
    import itertools
    best = (None, None, -1)
    for hand in itertools.combinations(deck8, 4):
        rest = [c for c in deck8 if c not in hand]
        for queue in itertools.permutations(rest):
            h, q, n = set(hand), list(queue), 0
            for c in plays:
                if c not in h:
                    break
                h.remove(c); q.append(c); h.add(q.pop(0)); n += 1
            if n > best[2]:
                best = (list(hand), list(queue), n)
    return best


TOWER_TROOP_IDS = {"tower-princess": 159000000, "princess-tower": 159000000, "cannoneer": 159000001,
                   "dagger-duchess": 159000002, "royal-chef": 159000004, "chef-tower": 159000004}


# ---------------------------------------------------------------------------
# 对局规格（spec）：两种数据源都先转成它，再生成原生 JSON
#   spec = {"sides": {0: S, 1: S}, "plays": [...], "abilities": [...], "meta": {...}}
#   S = {"name", "deck": [8 个 slug 或 None], "forms": {slug: el}, "levels": {slug: 等级}, "tower": 塔兵 id}
#   play = {"side", "tick", "card": slug, "x", "y"}（游戏内坐标）
#   ability = {"side", "tick", "candidates": [slug...]}
# ---------------------------------------------------------------------------
def spec_from_royaleapi(path: str | Path, forms: dict[int, dict[str, int]] | None = None) -> dict:
    """我们自己抓的 RoyaleAPI 回放：没有卡组和形态信息。卡组由出牌推断（不足 8 张用填充卡）。
    严格保留原局方位：side 按 data-i 标记（team = 1 - data_i），坐标原样（旋转/镜像会改变结局，见 re_notes）。
    forms 以 RoyaleAPI 方 "team"/"opponent" 或 "hog" 为键；缺省时野猪 2.6 一方用 HOG26_FORMS。"""
    r = load_replay(path)
    hog_side = next((s for s in ("team", "opponent")
                     if set((r.team if s == "team" else r.opponent).card_counts) == set(HOG26)), None)
    marks = {p.flag_i for p in r.plays}
    if len(marks) != 1:
        raise ValueError(f"data-i 标记不唯一：{marks}")
    data_i = next(iter(marks))
    side_of = {"team": 1 - data_i, "opponent": data_i}
    if forms is None:
        forms = {"hog": HOG26_FORMS}
    forms = {side_of[hog_side] if k == "hog" else side_of.get(k, k): v
             for k, v in forms.items() if not (k == "hog" and hog_side is None)}
    flip = False
    sides, skipped = {}, []
    for rside, side in side_of.items():
        seq = [x.card for x in r.side_plays(rside, include_abilities=False)]
        skipped += [c for c in dict.fromkeys(seq) if c not in cards.CARDS]
        deck = [c for c in dict.fromkeys(seq) if c in cards.CARDS][:8]
        for fl in FILLERS:
            if len(deck) >= 8:
                break
            if fl not in deck:
                deck.append(fl)
        sides[side] = {"name": (r.team if rside == "team" else r.opponent).name or f"side{side}", "deck": deck,
                       "forms": dict(forms.get(side, {})), "levels": {}, "tower": 159000000}
    plays, abilities = [], []
    for p in sorted(r.plays, key=lambda p: p.tick):
        side = side_of[p.side]
        if p.is_ability:
            abilities.append({"side": side, "tick": p.tick, "candidates": []})
        elif p.card in cards.CARDS:
            plays.append({"side": side, "tick": p.tick, "card": p.card, "x": p.x, "y": p.y})
    return {"sides": sides, "plays": plays, "abilities": abilities,
            "meta": {"source": str(path), "hog_side": hog_side, "hog_owner": side_of.get(hog_side),
                     "flip": flip, "unknown_cards": skipped, "side_of": side_of}}


def _split_form(key: str) -> tuple[str, int]:
    k = key.strip().lower()
    if k.endswith("-hero"):
        return k[:-5], 2
    for suf in ("-ev1", "-ev2"):
        if k.endswith(suf):
            return k[: -len(suf)], 1
    return k, 0


def spec_from_hf(payload: dict) -> dict:
    """FirstLight HF 数据集（royaleapi-battle-actions.v1）：卡组带形态和等级，有塔兵、技能候选。
    游戏内 side 按 data_i 标记：team = 1 - data_i（与 FirstLight royaleapi_replay.py 一致），坐标用 native_world_units。"""
    battle = payload["battle"]
    marks = {e.get("source_fields", {}).get("data_i") for e in payload["events"]} - {None}
    if len(marks) != 1:
        raise ValueError(f"data_i 标记不唯一：{marks}")
    data_i = next(iter(marks))
    side_of = {"team": 1 - data_i, "opponent": data_i}
    sides, skipped = {}, []
    for rside, side in side_of.items():
        pl = battle[rside]["players"]
        if len(pl) != 1:
            raise ValueError("不是 1v1")
        p = pl[0]
        deck, forms, levels = [], {}, {}
        for c in p["deck"]:
            slug, el = _split_form(c["card_key"])
            if slug not in cards.CARDS:
                skipped.append(slug)
                continue
            deck.append(slug)
            if el:
                forms[slug] = el
            if c.get("level"):
                levels[slug] = int(c["level"])
        tower = TOWER_TROOP_IDS.get((p.get("tower_card") or {}).get("card_key", "tower-princess"), 159000000)
        sides[side] = {"name": p.get("name") or f"side{side}", "deck": deck, "forms": forms, "levels": levels,
                       "tower": tower}
    plays, abilities = [], []
    for e in sorted(payload["events"], key=lambda e: e["replay_tick_20hz"]):
        side = side_of[e["side"]]
        if e["kind"] == "activate_ability":
            cands = [_split_form(c)[0] for c in (e.get("ability_source_candidates") or [])]
            abilities.append({"side": side, "tick": int(e["replay_tick_20hz"]), "candidates": cands})
        elif e["kind"] == "play_card":
            slug = _split_form(e["card_key"])[0]
            # 用原始 RoyaleAPI 坐标：native_world_units 多做了一次水平镜像，实测一致率 3/20，原始坐标 9/20
            raw = (e.get("coordinates") or {}).get("raw_royaleapi_units")
            if slug not in cards.CARDS or not raw:
                skipped.append(slug)
                continue
            plays.append({"side": side, "tick": int(e["replay_tick_20hz"]), "card": slug, "x": raw["x"], "y": raw["y"]})
    hog = [s for s in (0, 1) if set(sides[s]["deck"]) == set(HOG26)]
    return {"sides": sides, "plays": plays, "abilities": abilities,
            "meta": {"source": payload.get("source", {}).get("replay_tag"), "hog_owner": hog[0] if hog else None,
                     "unknown_cards": sorted(set(skipped)), "side_of": side_of,
                     "orig_crowns": {side_of["team"]: battle["team"]["crowns"],
                                     side_of["opponent"]: battle["opponent"]["crowns"]}}}


def level_index(slug: str, level: int | None) -> int:
    """卡牌等级 → 游戏内等级索引（level16_index 是 16 级的索引）。"""
    return cards.level16_index(slug) - (16 - level) if level else cards.level16_index(slug)


def build_from_spec(spec: dict, ability_gids: dict[int, int] | None = None) -> tuple[dict, dict]:
    ability_gids = ability_gids or {}
    out = copy.deepcopy(json.loads(TEMPLATE.read_text(encoding="utf-8")))
    b = out["battle"]
    assert out["rndSeed"] == SEED
    acct = {0: b["avatar0"]["accountID.lo"], 1: b["avatar1"]["accountID.lo"]}
    positions, legal = {}, {}
    for side in (0, 1):
        S = spec["sides"][side]
        deck = list(S["deck"])
        seq = [p["card"] for p in spec["plays"] if p["side"] == side]
        for c in dict.fromkeys(seq):          # 出过但卡组里没有的卡（数据异常）补进去
            if c not in deck:
                deck.append(c)
        for fl in FILLERS:
            if len(deck) >= 8:
                break
            if fl not in deck:
                deck.append(fl)
        deck = deck[:8]
        hand, queue, legal_n = best_order(deck, [c for c in seq if c in deck])
        legal[side] = (legal_n, len(seq))
        slots = [None] * 8
        hp, qp = SEED_PERM[side]
        for pos, slug in zip(hp + qp, hand + queue):
            slots[pos] = slug
        positions[side] = slots
        sp = []
        for s in slots:
            e = {"d": cards.card_id(s), "l": level_index(s, S["levels"].get(s))}
            if S["forms"].get(s):
                e["el"] = S["forms"][s]
            sp.append(e)
        b[f"deck{side}"]["sp"] = sp
        b[f"deck{side}"]["sc"][0]["d"] = S["tower"]
        b[f"avatar{side}"]["name"] = S["name"]

    cmds, enc = [], PdEncoder()
    for p in spec["plays"]:
        side = p["side"]
        if p["card"] not in positions[side]:
            continue
        d = positions[side].index(p["card"])
        pd = enc.encode(side, p["card"], d, spec["sides"][side]["forms"].get(p["card"], 0))
        cmds.append({"ct": PLAY_CT, "c": {"t": p["tick"] - COMMAND_AGE, "t2": p["tick"], "idHi": 0, "idLo": acct[side],
                                         "px": p["x"], "py": p["y"], "sid": -1,
                                         "sel": {"os": cards.card_id(p["card"]), "pd": pd}}})
    n_plays = len(cmds)
    abilities = [{"side": a["side"], "t2": a["tick"], "candidates": a["candidates"]} for a in spec["abilities"]]
    for i, a in enumerate(abilities):
        if ability_gids.get(i) is not None:
            cmds.append({"ct": ABILITY_CT, "c": {"t": a["t2"] - COMMAND_AGE, "t2": a["t2"], "idHi": 0,
                                                 "idLo": acct[a["side"]], "cgid": int(ability_gids[i])}})
    cmds.sort(key=lambda c: c["c"]["t2"])
    out["cmd"] = cmds
    out["evt"] = []
    out["endTick"] = max((c["c"]["t2"] for c in cmds), default=0) + 200
    used = sum(1 for v in ability_gids.values() if v is not None)
    meta = dict(spec["meta"], plays=n_plays, abilities=abilities, abilities_used=used,
                abilities_dropped=len(abilities) - used, legal_prefix=legal,
                forms={s: spec["sides"][s]["forms"] for s in (0, 1)}, decks=positions)
    return out, meta


def build(path: str | Path, forms: dict[int, dict[str, int]] | None = None,
          ability_gids: dict[int, int] | None = None) -> tuple[dict, dict]:
    """我们自己的 RoyaleAPI 回放 → 原生 JSON（兼容旧接口）。"""
    return build_from_spec(spec_from_royaleapi(path, forms), ability_gids)


def main(src: str, dst: str | None = None) -> Path:
    out, meta = build(src)
    dst_p = Path(dst) if dst else ROOT / "data/native_replays" / Path(src).name
    dst_p.parent.mkdir(parents=True, exist_ok=True)
    dst_p.write_text(json.dumps(out, ensure_ascii=False), encoding="utf-8")
    print(json.dumps(meta, ensure_ascii=False))
    return dst_p


if __name__ == "__main__":
    main(*sys.argv[1:])
