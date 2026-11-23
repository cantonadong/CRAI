"""Parse RoyaleAPI replay payloads (the JSON returned by /data/replay).

The payload is ``{"success": true, "html": "<div class='battle_replay' ...>"}``.
Each card play is a ``.replay_map .markers > div.marker`` element:

    data-x / data-y  arena position in 1/1000 tile (x: 0-17999, y: 0-31999), "None" for abilities
    data-c           card slug ("_invalid" for champion abilities)
    data-t           battle tick
    data-s           "t" = team (the player whose page it was), "o" = opponent
    data-i           unknown flag (always 0 in the samples so far)
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field, asdict
from pathlib import Path

from selectolax.lexbor import LexborHTMLParser as HTMLParser

TILE = 1000  # replay coordinate units per arena tile
ABILITY_SLUG = "_invalid"


@dataclass(frozen=True)
class Play:
    tick: int
    side: str  # "team" | "opponent"
    card: str
    x: int | None
    y: int | None
    is_ability: bool
    flag_i: int

    @property
    def tile(self) -> tuple[int, int] | None:
        if self.x is None or self.y is None:
            return None
        return self.x // TILE, self.y // TILE


@dataclass
class SideStats:
    name: str = ""
    card_counts: dict[str, int] = field(default_factory=dict)
    elixir: dict[str, float] = field(default_factory=dict)  # total/troop/building/spell/ability/leaked
    counts: dict[str, int] = field(default_factory=dict)


@dataclass
class Replay:
    battle_tag: str
    plays: list[Play]
    team: SideStats
    opponent: SideStats

    def side_plays(self, side: str, include_abilities: bool = True) -> list[Play]:
        return [p for p in self.plays if p.side == side and (include_abilities or not p.is_ability)]

    def to_rows(self) -> list[dict]:
        return [{"battle_tag": self.battle_tag, **asdict(p)} for p in self.plays]


def _int_or_none(v: str | None) -> int | None:
    if v is None or v == "None" or v == "":
        return None
    return int(v)


def _num(text: str) -> float:
    return float(text.strip() or 0)


def _parse_elixir_table(table) -> tuple[dict[str, float], dict[str, int]]:
    elixir: dict[str, float] = {}
    counts: dict[str, int] = {}
    for tr in table.css("tr"):
        title = tr.css_first("td.title")
        if title is None:
            continue
        key = title.text(strip=True).lower()
        count_td = tr.css_first("td.count")
        elixir_td = tr.css_first("td.elixir")
        if count_td is not None:
            counts[key] = int(_num(count_td.text()))
        if elixir_td is not None:
            elixir[key] = _num(elixir_td.text())
    return elixir, counts


def parse_replay_html(html: str) -> Replay:
    tree = HTMLParser(html)
    root = tree.css_first(".battle_replay")
    battle_tag = root.attributes.get("data-tag", "") if root else ""

    plays: list[Play] = []
    for m in tree.css(".replay_map .markers > div.marker"):
        a = m.attributes
        card = a.get("data-c", "")
        plays.append(
            Play(
                tick=int(a["data-t"]),
                side="team" if a.get("data-s") == "t" else "opponent",
                card=card,
                x=_int_or_none(a.get("data-x")),
                y=_int_or_none(a.get("data-y")),
                is_ability=card == ABILITY_SLUG,
                flag_i=int(a.get("data-i") or 0),
            )
        )
    plays.sort(key=lambda p: p.tick)

    sides = [SideStats(), SideStats()]
    columns = tree.css(".stats .column.pad0topbottom")
    for side, col in zip(sides, columns):
        h = col.css_first("h5")
        side.name = h.text(strip=True) if h else ""
        for item in col.css(".replay_stats .item"):
            img = item.css_first("img.replay_card")
            cnt = item.css_first(".card_count")
            if img is not None and cnt is not None:
                side.card_counts[img.attributes.get("data-card", "")] = int(_num(cnt.text()))
    for side, table in zip(sides, tree.css("table.replay_elixir_table")):
        side.elixir, side.counts = _parse_elixir_table(table)

    return Replay(battle_tag=battle_tag, plays=plays, team=sides[0], opponent=sides[1])


def parse_replay_payload(payload: str | bytes | dict) -> Replay:
    if not isinstance(payload, dict):
        payload = json.loads(payload)
    if not payload.get("success"):
        raise ValueError("replay payload has success=false")
    return parse_replay_html(payload["html"])


def load_replay(path: str | Path) -> Replay:
    return parse_replay_payload(Path(path).read_text(encoding="utf-8"))


if __name__ == "__main__":
    import sys

    r = load_replay(sys.argv[1])
    print(f"battle {r.battle_tag}: {r.team.name!r} vs {r.opponent.name!r}, {len(r.plays)} plays")
    for p in r.plays:
        print(f"  t={p.tick:5d} {p.side:8s} {p.card:20s} tile={p.tile}")
