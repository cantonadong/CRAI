"""跨版本函数匹配（15.535.13 → 16.635.1），简化版 BinDiff。

输入：两份 ExportFuncFeatures.java 导出的 JSONL。
步骤：
  1. 锚点：特征完全一致且两边都唯一的函数（字符串集合、助记符哈希+指令数、常量集合）。
  2. 传播：已匹配的函数对，按调用顺序对齐被调函数；同时对齐调用者。只接受相似度够高的对。
  3. 反复传播直到不再增加。
输出：re/fl_funcmap.json（旧入口 → 新入口、相似度、来源），以及 FirstLight probe 里各偏移的映射结果。

用法：python scripts/fl_match.py re/feat_15535.jsonl re/feat_16635.jsonl
"""
import json
import re
import sys
from collections import Counter, defaultdict
from difflib import SequenceMatcher
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def load(path):
    funcs = {}
    for line in open(path, encoding="utf-8"):
        f = json.loads(line)
        f["mnl"] = f["mn"].split()
        funcs[f["e"]] = f
    callers = defaultdict(list)
    for e, f in funcs.items():
        for c in f["c"]:
            if c in funcs:
                callers[c].append(e)
    for e in callers:
        callers[e].sort()
    return funcs, callers


def sim(a, b):
    """0..1 相似度：助记符序列 + 大小 + 常量 + 字符串。"""
    if a["m"] == b["m"] and a["ic"] == b["ic"]:
        s_mn = 1.0
    else:
        s_mn = SequenceMatcher(None, a["mnl"], b["mnl"], autojunk=False).ratio()
    na, nb = a["ic"] or 1, b["ic"] or 1
    s_sz = min(na, nb) / max(na, nb)
    ka, kb = Counter(a["k"]), Counter(b["k"])
    s_k = (sum((ka & kb).values()) * 2 / (sum(ka.values()) + sum(kb.values()))) if (ka or kb) else 1.0
    sa, sb = Counter(a["s"]), Counter(b["s"])
    s_s = (sum((sa & sb).values()) * 2 / (sum(sa.values()) + sum(sb.values()))) if (sa or sb) else 1.0
    return 0.45 * s_mn + 0.2 * s_sz + 0.2 * s_k + 0.15 * s_s


def unique_keys(funcs, keyfn):
    groups = defaultdict(list)
    for e, f in funcs.items():
        k = keyfn(f)
        if k is not None:
            groups[k].append(e)
    return {k: v[0] for k, v in groups.items() if len(v) == 1}


def main(old_path, new_path):
    old, old_callers = load(old_path)
    new, new_callers = load(new_path)
    match = {}       # old -> (new, score, how)
    used = set()

    def accept(o, n, score, how):
        if o in match or n in used:
            return False
        match[o] = (n, score, how)
        used.add(n)
        return True

    keyfns = [
        ("str", lambda f: tuple(sorted(f["s"])) if len(f["s"]) >= 2 or (f["s"] and len(f["s"][0]) >= 8) else None),
        ("mnhash", lambda f: (f["m"], f["ic"]) if f["ic"] >= 24 else None),
        ("const", lambda f: tuple(sorted(f["k"])) if len(f["k"]) >= 4 else None),
    ]
    for name, fn in keyfns:
        uo, un = unique_keys(old, fn), unique_keys(new, fn)
        n0 = len(match)
        for k, o in uo.items():
            n = un.get(k)
            if n is not None:
                s = sim(old[o], new[n])
                if s >= 0.6:
                    accept(o, n, s, name)
        print(f"anchor {name}: +{len(match) - n0}")

    # 传播
    for rnd in range(30):
        added = 0
        for o, (n, _, _) in list(match.items()):
            for lo, ln in ((old[o]["c"], new[n]["c"]), (old_callers.get(o, []), new_callers.get(n, []))):
                lo = [x for x in dict.fromkeys(lo) if x in old and x not in match]
                ln = [x for x in dict.fromkeys(ln) if x in new and x not in used]
                if not lo or not ln:
                    continue
                if len(lo) == len(ln):
                    for a, b in zip(lo, ln):
                        s = sim(old[a], new[b])
                        if s >= 0.7 and accept(a, b, s, f"prop{rnd}"):
                            added += 1
                else:
                    # 长度不同：每个旧函数在候选里取最相似的，要求明显领先
                    if len(lo) * len(ln) > 400:
                        continue
                    for a in lo:
                        scored = sorted(((sim(old[a], new[b]), b) for b in ln if b not in used), reverse=True)
                        if not scored:
                            continue
                        top = scored[0]
                        second = scored[1][0] if len(scored) > 1 else 0
                        if top[0] >= 0.8 and top[0] - second >= 0.1 and accept(a, top[1], top[0], f"propb{rnd}"):
                            added += 1
        print(f"round {rnd}: +{added} total {len(match)}")
        if not added:
            break

    out = ROOT / "re/fl_funcmap.json"
    out.write_text(json.dumps({hex(o): [hex(n), round(s, 3), h] for o, (n, s, h) in sorted(match.items())}, indent=0))
    print(f"matched {len(match)}/{len(old)} -> {out}")

    # FirstLight probe 偏移
    probe_dir = ROOT / "refs/FirstLight_CR/native_runner/probe"
    rows = []
    seen = set()
    for p in sorted(probe_dir.glob("*")):
        if p.suffix not in {".cpp", ".h", ".inc"} or p.name.startswith("test_"):
            continue
        for name, val in re.findall(r"k(\w+?Offset)\s*=\s*(0x[0-9a-fA-F]{5,})", p.read_text(encoding="utf-8", errors="ignore")):
            if name in seen:
                continue
            seen.add(name)
            v = int(val, 16)
            if v in match:
                n, s, h = match[v]
                rows.append((name, v, n, s, h))
            elif v in old:
                rows.append((name, v, None, 0, "func-unmatched"))
            else:
                rows.append((name, v, None, 0, "not-func-entry"))
    res = ROOT / "re/fl_probe_offsets.tsv"
    with open(res, "w", encoding="utf-8") as fh:
        for name, v, n, s, h in rows:
            fh.write(f"{name}\t{v:#x}\t{'' if n is None else hex(n)}\t{s:.3f}\t{h}\n")
    c = Counter(r[4] if r[2] is None else "matched" for r in rows)
    print("probe offsets:", dict(c), "->", res)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
