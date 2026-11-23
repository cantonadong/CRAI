from collections import Counter
from pathlib import Path

from crai.data.replay import load_replay

FIXTURE = Path(__file__).parent / "fixtures" / "replay1.json"


def test_parse_replay1():
    r = load_replay(FIXTURE)
    assert r.battle_tag == "002YQR8QVJY8"

    team_cards = r.side_plays("team", include_abilities=False)
    opp_cards = r.side_plays("opponent", include_abilities=False)
    # Must match the page's elixir tables ("Total" counts) and ability counts.
    assert len(team_cards) == r.team.counts["total"] == 21
    assert len(opp_cards) == r.opponent.counts["total"] == 21
    assert sum(p.is_ability for p in r.side_plays("team")) == r.team.counts["ability"] == 1
    assert sum(p.is_ability for p in r.side_plays("opponent")) == r.opponent.counts["ability"] == 2

    # Per-card counts must match the stats panel.
    assert Counter(p.card for p in team_cards) == r.team.card_counts
    assert Counter(p.card for p in opp_cards) == r.opponent.card_counts

    assert r.team.elixir["total"] == 87 and r.team.elixir["leaked"] == 0.66
    assert r.opponent.elixir["total"] == 84


def test_play_fields():
    r = load_replay(FIXTURE)
    first = r.plays[0]
    assert (first.tick, first.side, first.card, first.x, first.y) == (174, "team", "barbarian-barrel", 3500, 17500)
    assert first.tile == (3, 17)
    assert [p.tick for p in r.plays] == sorted(p.tick for p in r.plays)
    for p in r.plays:
        if p.is_ability:
            assert p.x is None and p.tile is None
        else:
            assert 0 <= p.x < 18000 and 0 <= p.y < 32000
