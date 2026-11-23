"""对 fl_map_addrs.py 结果里不确定的代码地址做二次定位。

方法：
  1. 调用点投票：旧版里所有 BL/B 指向目标 X 的位置，所在函数若已匹配，就把调用指令按指令序列
     对齐到新版函数，读出新版调用指令的目标，投票。
  2. 虚表槽位：旧版 RELATIVE 重定位里值为 X 的槽位，所在虚表若能映射到新版，读新版同一槽位。
  3. 导入符号：PLT 跳板（按 GOT 槽对应的符号名）和 GOT 槽（按符号名）直接查表。
  4. 调用点本身（如 TranslatedSha256InitializerCall）：先映射它调用的目标函数，再在新版里找调用该目标的指令。
用法：python scripts/fl_refine.py [名字 ...]   （不给名字就处理 conf < 0.9 的全部代码地址）
"""
import csv
import struct
import sys
from collections import Counter, defaultdict
from difflib import SequenceMatcher
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fl_map_addrs as M  # noqa: E402

ROOT = M.ROOT


class Elf:
    def __init__(self, lib):
        self.lib = lib
        b = lib.b
        # section header 被剥掉了，走 PT_DYNAMIC
        phoff, = struct.unpack_from("<Q", b, 0x20)
        n, = struct.unpack_from("<H", b, 0x38)
        dyn = {}
        for i in range(n):
            t, fl, off, va, pa, fs, ms, al = struct.unpack_from("<IIQQQQQQ", b, phoff + i * 56)
            if t == 2:
                for j in range(0, fs, 16):
                    tag, val = struct.unpack_from("<qQ", b, off + j)
                    if tag == 0:
                        break
                    dyn.setdefault(tag, val)

        def foff(va):
            for v, ms, off, fs, fl in lib.loads:
                if v <= va < v + fs:
                    return off + va - v
            raise ValueError(hex(va))

        symtab, strtab = foff(dyn[6]), foff(dyn[5])

        class Syms:
            def __getitem__(_, i):
                st_name, = struct.unpack_from("<I", b, symtab + i * 24)
                s = strtab + st_name
                return b[s:b.index(b"\0", s)].decode(errors="replace")

        self.syms = Syms()
        self.relative = defaultdict(list)   # value -> [slot]
        self.slot_value = {}
        self.got_sym = {}                   # got slot -> symbol name
        self.secs = {}
        tables = [(dyn.get(7), dyn.get(8, 0)), (dyn.get(23), dyn.get(2, 0))]
        for start, size in tables:
            if not start:
                continue
            base = foff(start)
            for i in range(size // 24):
                off, info, add = struct.unpack_from("<QQq", b, base + i * 24)
                typ, sym = info & 0xFFFFFFFF, info >> 32
                if typ == 1027:   # R_AARCH64_RELATIVE
                    self.relative[add].append(off)
                    self.slot_value[off] = add
                elif typ in (1025, 1026, 257):  # GLOB_DAT, JUMP_SLOT, ABS64
                    self.got_sym[off] = self.syms[sym]
        self.sym_got = {v: k for k, v in self.got_sym.items()}
        # 全部 BL / B 指令（numpy 快速解码）
        text = np.frombuffer(b[: lib.text_end - lib.text_end % 4], dtype="<u4")
        op = text & 0xFC000000
        idx = np.nonzero((op == 0x94000000) | (op == 0x14000000))[0]
        imm = (text[idx] & 0x03FFFFFF).astype(np.int64)
        imm = np.where(imm & 0x02000000, imm - 0x04000000, imm)
        self.br_src = idx * 4
        self.br_dst = self.br_src + imm * 4
        # PLT 跳板：... br x17（0xd61f0220），开头是 adrp x16
        self.plt = {}
        for i in np.nonzero(text == 0xD61F0220)[0].tolist():
            a = i * 4 - 12
            got, sym = self.plt_target_symbol(a)
            if got is not None:
                self.plt[a] = (got, sym)
        self.plt_by_sym = {sym: a for a, (got, sym) in self.plt.items() if sym}

    def callers_of(self, target):
        return self.br_src[self.br_dst == target].tolist()

    def plt_target_symbol(self, addr):
        """PLT 跳板：adrp x16, page; ldr x17, [x16, #off]; ... → GOT 槽 → 符号名。"""
        ins = self.lib.insns(addr, 16)
        if len(ins) < 2 or ins[0][1] != "adrp" or ins[1][1] != "ldr" or not ins[0][2].startswith("x16"):
            return None, None
        try:
            page = int(ins[0][2].split("#")[1], 16)
            off = int(ins[1][2].split("#")[1].rstrip("]"), 16)
        except (IndexError, ValueError):
            return None, None
        got = page + off
        return got, self.got_sym.get(got)

    def plt_for_symbol(self, sym):
        return self.plt_by_sym.get(sym)


old_elf, new_elf = Elf(M.old), Elf(M.new)


def map_call_insn(src_old):
    """旧版的一条调用指令 → 新版对应的调用指令地址（通过所在函数的指令对齐）。"""
    f = M.old.containing(src_old)
    if f is None or f not in M.fmap:
        return None
    n, _, _ = M.align_inside(f, M.fmap[f][0], src_old)
    return n


def decode_branch(lib, addr):
    w, = struct.unpack_from("<I", lib.b, addr)
    if w & 0xFC000000 not in (0x94000000, 0x14000000):
        return None
    imm = w & 0x03FFFFFF
    if imm & 0x02000000:
        imm -= 0x04000000
    return addr + imm * 4


def vote_by_callers(x):
    votes = Counter()
    for src in old_elf.callers_of(x):
        dst = map_call_insn(src)
        if dst is None:
            continue
        t = decode_branch(M.new, dst)
        if t is not None:
            votes[t] += 1
    return votes


def vote_by_vtables(x):
    votes = Counter()
    for slot in old_elf.relative.get(x, []):
        # 往前找虚表起点：被数据投票映射过的地址
        for back in range(0, 0x400, 8):
            base = slot - back
            nb, conf, _ = M.map_data(base)
            if nb is not None and conf >= 0.8:
                v = new_elf.slot_value.get(nb + back)
                if v is not None:
                    votes[v] += 1
                break
    return votes


def refine(name, old_addr):
    notes = []
    # 导入符号：GOT 槽
    if old_addr in old_elf.got_sym:
        sym = old_elf.got_sym[old_addr]
        return new_elf.sym_got.get(sym), 1.0, f"got:{sym}"
    # PLT 跳板
    if old_addr in old_elf.plt:
        _, sym = old_elf.plt[old_addr]
        return new_elf.plt_for_symbol(sym), 1.0, f"plt:{sym}"
    # 调用点本身
    t_old = decode_branch(M.old, old_addr)
    if t_old is not None and "Call" in name:
        t_new, conf, how = refine("_target", t_old)
        if t_new is not None:
            srcs = new_elf.callers_of(t_new)
            near = map_call_insn(old_addr)
            if near in srcs:
                return near, conf, f"call-site(target {how})"
            if len(srcs) == 1:
                return srcs[0], conf * 0.9, f"unique-caller-of({t_new:#x},{how})"
            notes.append(f"target {t_new:#x} has {len(srcs)} callers")
    v = vote_by_callers(old_addr) + vote_by_vtables(old_addr)
    if v:
        (best, k), total = v.most_common(1)[0], sum(v.values())
        if old_addr in M.old.funcs and best in M.new.funcs:
            s, _, _ = M.func_sim(old_addr, best)
        else:
            a = [M.norm(i) for i in M.old.insns(old_addr, 0x200)]
            bb = [M.norm(i) for i in M.new.insns(best, 0x200)]
            s = SequenceMatcher(None, a, bb, autojunk=False).ratio()
        return best, min(1.0, k / total) * (0.5 + 0.5 * s), f"xref-vote({k}/{total},sim={s:.2f})"
    return None, 0, "unresolved " + "; ".join(notes)


def main():
    rows = list(csv.DictReader(open(ROOT / "re/fl_addr_map.tsv", encoding="utf-8"), delimiter="\t"))
    want = set(sys.argv[1:])
    for r in rows:
        if want and r["name"] not in want:
            continue
        if not want and float(r["conf"]) >= 0.9:
            continue
        n, conf, how = refine(r["name"], int(r["old"], 16))
        prev = r["new"] or "-"
        print(f"{r['name']:45s} old={r['old']:>10s} prev={prev:>10s}({float(r['conf']):.2f}) -> "
              f"{'-' if n is None else hex(n):>10s} conf={conf:.2f} {how}")


if __name__ == "__main__":
    main()
