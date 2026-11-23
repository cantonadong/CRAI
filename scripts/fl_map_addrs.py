"""把 FirstLight probe 里的全部地址（函数、函数内部位置、虚表、全局变量）映射到 16.635.1。

依赖 scripts/fl_match.py 的输出 re/fl_funcmap.json 和两份特征 re/feat_*.jsonl。

方法：
  - 函数入口：用函数匹配结果，再用 capstone 对整个函数做指令级比对，给出置信度。
  - 函数内部位置（调用点、返回点）：找到所在的旧函数及其新函数，按指令序列对齐，取对应指令。
  - 没有函数匹配的代码地址：取前后最近的可靠匹配函数，按比例推到新版的一个窗口，
    在窗口内找开头指令最相似的函数入口。
  - 数据地址（虚表、全局变量）：所有引用它的已匹配函数，按引用顺序对齐后投票。
输出：re/fl_addr_map.tsv（名字、旧地址、新地址、方法、置信度、备注）。
"""
import bisect
import json
import re
import struct
from collections import Counter, defaultdict
from difflib import SequenceMatcher
from pathlib import Path

import capstone

ROOT = Path(__file__).resolve().parents[1]
OLD_LIB = ROOT / "apk/libs_15535/libg.so"
NEW_LIB = ROOT / "apk/libs/arm64-v8a/libg.so"
PROBE = ROOT / "refs/FirstLight_CR/native_runner/probe"

cs = capstone.Cs(capstone.CS_ARCH_ARM64, capstone.CS_MODE_ARM)


class Lib:
    def __init__(self, path, feat):
        self.b = path.read_bytes()
        phoff, = struct.unpack_from("<Q", self.b, 0x20)
        n, = struct.unpack_from("<H", self.b, 0x38)
        self.loads = []
        for i in range(n):
            t, fl, off, va, pa, fs, ms, al = struct.unpack_from("<IIQQQQQQ", self.b, phoff + i * 56)
            if t == 1:
                self.loads.append((va, ms, off, fs, fl))
        self.funcs = {}
        for line in open(feat, encoding="utf-8"):
            f = json.loads(line)
            self.funcs[f["e"]] = f
        self.entries = sorted(self.funcs)
        self.text_end = self.loads[0][1]

    def read(self, va, n):
        for v, ms, off, fs, fl in self.loads:
            if v <= va < v + fs:
                return self.b[off + va - v: off + va - v + n]
        return b""

    def is_code(self, va):
        return va < self.text_end

    def containing(self, va):
        i = bisect.bisect_right(self.entries, va) - 1
        if i < 0:
            return None
        e = self.entries[i]
        return e if va < e + self.funcs[e]["n"] else None

    def insns(self, start, size):
        return [(i.address, i.mnemonic, i.op_str) for i in cs.disasm(self.read(start, size), start)]


def norm(ins):
    """指令归一化：保留助记符和寄存器，去掉立即数和地址。"""
    _, m, ops = ins
    ops = re.sub(r"#-?0x[0-9a-f]+|#-?\d+", "#", ops)
    return m + " " + ops


old = Lib(OLD_LIB, ROOT / "re/feat_15535.jsonl")
new = Lib(NEW_LIB, ROOT / "re/feat_16635.jsonl")
fmap = {int(o, 16): (int(n, 16), s, h) for o, (n, s, h) in json.loads((ROOT / "re/fl_funcmap.json").read_text()).items()}


def func_sim(o, n):
    a = [norm(x) for x in old.insns(o, old.funcs[o]["n"])]
    b = [norm(x) for x in new.insns(n, new.funcs[n]["n"])]
    return SequenceMatcher(None, a, b, autojunk=False).ratio(), a, b


def align_inside(o_func, n_func, addr):
    oi = old.insns(o_func, old.funcs[o_func]["n"])
    ni = new.insns(n_func, new.funcs[n_func]["n"])
    a = [norm(x) for x in oi]
    b = [norm(x) for x in ni]
    idx = next((i for i, x in enumerate(oi) if x[0] == addr), None)
    if idx is None:
        return None, 0, "addr-not-insn"
    sm = SequenceMatcher(None, a, b, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if i1 <= idx < i2:
            if tag == "equal":
                return ni[j1 + idx - i1][0], sm.ratio(), "insn-equal"
            if tag == "replace" and (i2 - i1) == (j2 - j1):
                return ni[j1 + idx - i1][0], sm.ratio() * 0.8, "insn-replace"
            return None, sm.ratio(), f"insn-{tag}"
    return None, 0, "align-fail"


reliable = sorted((o, n) for o, (n, s, h) in fmap.items() if s >= 0.95)
rel_old = [o for o, _ in reliable]


def window_search(addr):
    i = bisect.bisect_right(rel_old, addr) - 1
    if i < 0 or i + 1 >= len(reliable):
        return None, 0, "no-neighbors"
    (o0, n0), (o1, n1) = reliable[i], reliable[i + 1]
    lo, hi = min(n0, n1), max(n0, n1)
    if hi - lo > 0x20000:
        return None, 0, "window-too-wide"
    target = old.insns(addr, 0x100)
    a = [norm(x) for x in target]
    best = []
    for e in new.entries:
        if lo <= e <= hi:
            b = [norm(x) for x in new.insns(e, min(0x100, new.funcs[e]["n"]))]
            best.append((SequenceMatcher(None, a, b, autojunk=False).ratio(), e))
    # 窗口内也可能有 Ghidra 没识别成函数的位置：按旧版相对位置估一个点，前后找开头 16 字节
    pro = old.read(addr, 16)
    for m in re.finditer(re.escape(pro), new.b[lo:hi]):
        e = lo + m.start()
        if e % 4 == 0 and e not in new.funcs:
            b = [norm(x) for x in new.insns(e, 0x100)]
            best.append((SequenceMatcher(None, a, b, autojunk=False).ratio(), e))
    if not best:
        return None, 0, "window-empty"
    best.sort(reverse=True)
    margin = best[0][0] - (best[1][0] if len(best) > 1 else 0)
    return best[0][1], best[0][0], f"window(n={len(best)},margin={margin:.2f})"


# 数据地址投票：old_data -> Counter(new_data)
data_votes = defaultdict(Counter)
for o, (n, s, h) in fmap.items():
    if s < 0.85:
        continue
    do, dn = old.funcs[o]["d"], new.funcs[n]["d"]
    if len(do) == len(dn):
        for x, y in zip(do, dn):
            data_votes[x][y] += 1


def map_data(addr):
    if addr in data_votes:
        c = data_votes[addr]
        (best, k), total = c.most_common(1)[0], sum(c.values())
        return best, k / total, f"data-vote({k}/{total})"
    # 虚表常被引用为 vtable+0x10：找附近被引用的地址
    for d in range(8, 0x40, 8):
        for base in (addr - d, addr + d):
            if base in data_votes:
                c = data_votes[base]
                (best, k), total = c.most_common(1)[0], sum(c.values())
                return best + (addr - base), k / total * 0.9, f"data-vote-near({addr - base:+#x},{k}/{total})"
    return None, 0, "data-unreferenced"


def map_addr(addr):
    if not old.is_code(addr):
        return map_data(addr)
    if addr in fmap:
        n = fmap[addr][0]
        s, _, _ = func_sim(addr, n)
        return n, s, "func-entry"
    c = old.containing(addr)
    if c is not None and c != addr and c in fmap:
        return align_inside(c, fmap[c][0], addr)
    return window_search(addr)


def main():
    rows, seen = [], set()
    for p in sorted(PROBE.glob("*")):
        if p.suffix not in {".cpp", ".h", ".inc"} or p.name.startswith("test_"):
            continue
        for name, val in re.findall(r"k(\w+?Offset)\s*=\s*(0x[0-9a-fA-F]{5,})", p.read_text(encoding="utf-8", errors="ignore")):
            if name in seen:
                continue
            seen.add(name)
            v = int(val, 16)
            n, conf, how = map_addr(v)
            rows.append((name, v, n, conf, how, p.name))
    out = ROOT / "re/fl_addr_map.tsv"
    with open(out, "w", encoding="utf-8") as fh:
        fh.write("name\told\tnew\tconf\tmethod\tsource\n")
        for name, v, n, conf, how, src in rows:
            fh.write(f"{name}\t{v:#x}\t{'' if n is None else hex(n)}\t{conf:.3f}\t{how}\t{src}\n")
    ok = sum(1 for r in rows if r[2] is not None and r[3] >= 0.85)
    weak = sum(1 for r in rows if r[2] is not None and r[3] < 0.85)
    miss = sum(1 for r in rows if r[2] is None)
    print(f"{len(rows)} addrs: confident {ok}, weak {weak}, missing {miss} -> {out}")


if __name__ == "__main__":
    main()
