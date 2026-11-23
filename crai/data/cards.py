"""卡牌表：RoyaleAPI slug <-> 游戏卡牌 ID <-> 费用。

来源：refs/cr-memory-reader/deploy/cards.json（官方 API 格式的卡牌事实数据：名称、ID、费用），
生成后缓存到 configs/cards.json，项目内只依赖缓存文件。
slug 规则与 RoyaleAPI 回放一致：小写、空格和点号转为 '-'，例如 "The Log" -> "the-log"，"P.E.K.K.A" -> "pekka"。
"""

from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / "configs" / "cards.json"
SOURCE = ROOT / "refs" / "cr-memory-reader" / "deploy" / "cards.json"

# RoyaleAPI 与官方名称规则不一致的个例
SLUG_FIX = {"pekka": "pekka", "mini-pekka": "mini-pekka"}


def slugify(name: str) -> str:
    s = name.lower().replace(".", "").replace("'", "")
    s = re.sub(r"[^a-z0-9]+", "-", s).strip("-")
    return SLUG_FIX.get(s, s)


def build_cache() -> dict:
    d = json.loads(SOURCE.read_text(encoding="utf-8"))
    cards = {}
    for it in d["items"] + d.get("supportItems", []):
        cards[slugify(it["name"])] = {"id": it["id"], "cost": it.get("elixirCost", 0), "name": it["name"],
                                      "rarity": it.get("rarity", "common")}
    CACHE.write_text(json.dumps(cards, indent=1, ensure_ascii=False), encoding="utf-8")
    return cards


def load() -> dict:
    if CACHE.exists():
        return json.loads(CACHE.read_text(encoding="utf-8"))
    return build_cache()


CARDS = load()
ID2SLUG = {v["id"]: k for k, v in CARDS.items()}


def card_id(slug: str) -> int:
    return CARDS[slug]["id"] if slug in CARDS else -1


LEVEL16_INDEX = {"common": 15, "rare": 13, "epic": 10, "legendary": 7, "champion": 5}


def level16_index(slug: str) -> int:
    """满级（16 级）在卡组元素 +0x18 中的等级索引，按稀有度换算（已在内存中核对：普通15/稀有13/史诗10/传奇7）。"""
    return LEVEL16_INDEX.get(CARDS.get(slug, {}).get("rarity", "common"), 15)


def cost(slug_or_id) -> int:
    if isinstance(slug_or_id, int):
        slug_or_id = ID2SLUG.get(slug_or_id, "")
    return CARDS.get(slug_or_id, {}).get("cost", 0)
