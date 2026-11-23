"""PPO 对手卡组池：HF 数据集里所有野猪 2.6 对局的"对手方"卡组（真实天梯分布，带形态和塔兵）。

只保留 8 张牌都是我们引擎/卡牌表认识的卡组；按出现次数排序。输出 data/ppo/deck_pool.json：
  [{"deck": [8 个 slug], "forms": {slug: el}, "tower": 塔兵 ID, "count": 次数}, …]
用法：.venv\\Scripts\\python.exe scripts/build_deck_pool.py
"""
import json
import sys
from collections import Counter
from pathlib import Path

import pyarrow.parquet as pq

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import royaleapi_to_native as C  # noqa: E402
from hf_resim_hog import is_hog_payload  # noqa: E402

SRC = ROOT / "data/hf_il_replay/replays"
OUT = ROOT / "data/ppo/deck_pool.json"


def main():
    counter: Counter = Counter()
    n_games = n_bad = 0
    for part in sorted(SRC.glob("part-*.parquet")):
        t = pq.read_table(part, columns=["payload_json"])
        for pj in t.column("payload_json").to_pylist():
            p = json.loads(pj)
            if not is_hog_payload(p):
                continue
            n_games += 1
            try:
                spec = C.spec_from_hf(p)
            except Exception:  # noqa: BLE001
                n_bad += 1
                continue
            hog = spec["meta"]["hog_owner"]
            if hog is None or spec["meta"]["unknown_cards"]:
                n_bad += 1
                continue
            opp = spec["sides"][1 - hog]
            if len(opp["deck"]) != 8:
                n_bad += 1
                continue
            key = (tuple(sorted(opp["deck"])), tuple(sorted(opp["forms"].items())), opp["tower"])
            counter[key] += 1
        print(f"{part.name}: 累计野猪局 {n_games}，不同对手卡组 {len(counter)}", flush=True)
    pool = [{"deck": list(d), "forms": dict(f), "tower": tw, "count": c} for (d, f, tw), c in counter.most_common()]
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(pool, ensure_ascii=False), encoding="utf-8")
    print(f"野猪局 {n_games}，跳过 {n_bad}；对手卡组 {len(pool)} 种 → {OUT}")


if __name__ == "__main__":
    main()
