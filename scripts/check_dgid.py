"""核对 16.635.1 引擎里单位的数据全局 ID（dgid）是否都在 FirstLight 的 entity_archetype_catalog 里。
FirstLight 的 tensorizer 遇到不认识的 dgid 会直接报错，所以缺的要做映射。

用法：python scripts/check_dgid.py [局数 2]
"""
import gzip
import json
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.bot.rules import Hog26Bot  # noqa: E402
from crai.env import match as M  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402

CAT = ROOT / "refs/FirstLight_CR/native_runner/data/competitive/model_catalogs.json.gz"


class Recorder:
    """包装一个 AI，顺便记录每帧看到的单位 (card, dgid)。"""

    def __init__(self, inner, seen):
        self.inner, self.seen = inner, seen

    def decide(self, frame, now=None):
        return self.inner.decide(frame, now)


def main(n="2"):
    cat = json.load(gzip.open(CAT, "rt", encoding="utf-8"))["catalogs"]["entity_archetype_catalog"]
    known = {}
    for gid, name in cat["character_global_ids"]:
        known[int(gid)] = f"character:{name}"
    for key, node in zip(cat["archetype_keys"], cat["archetype_node_ids"]):
        kind, _, num = key.partition(":")
        if num.lstrip("-").isdigit():
            known.setdefault(int(num), f"{kind}:{node}")
    print(f"FirstLight 目录 {len(known)} 个单位原型")

    seen = defaultdict(set)   # dgid -> {card}
    env = NativeEnv()
    orig_step = env.step

    def step(t=1):
        f = orig_step(t)
        for e in f["entities"]:
            if e["card"] != -1:
                seen[e["dgid"]].add(e["card"])
        for e in f.get("others", []):
            seen[e["dgid"]].add(("other", e["card"], e["vt"]))
        return f

    env.step = step
    for g in range(int(n)):
        M.play_match(Hog26Bot(), Hog26Bot(), seed=2000 + g, env=env)
    env.close()
    ok = {k: v for k, v in seen.items() if k in known}
    miss = {k: v for k, v in seen.items() if k not in known}
    print(f"出现过的 dgid {len(seen)} 个：目录里有 {len(ok)}，没有 {len(miss)}")
    for k, v in sorted(ok.items()):
        print(f"  ✓ {k:12d} {known[k]!s:28s} {sorted(map(str, v))[:3]}")
    for k, v in sorted(miss.items()):
        print(f"  ✗ {k:12d} {sorted(map(str, v))[:3]}")


if __name__ == "__main__":
    main(*sys.argv[1:])
