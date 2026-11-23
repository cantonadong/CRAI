"""追踪 "adrp+ldr 全局指针 -> 逐级 ldr [reg,#off]" 的指令链，统计每个全局变量后面跟着的字段偏移序列。

用法: python -I scripts/re_global_chains.py <libg.so> 0x20,0x90 [最少命中]
输出: 以给定偏移序列开头的链，按全局地址分组计数。
"""
import re
import sys
from collections import Counter, defaultdict

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs

sys.path.insert(0, __file__.rsplit("\\", 1)[0].rsplit("/", 1)[0])
from re_patterns import exec_segments  # noqa: E402

ADRP = re.compile(r"^(x\d+), #(0x[0-9a-f]+)$")
LDR = re.compile(r"^(x\d+), \[(x\d+)(?:, #(0x[0-9a-f]+))?\]$")
ADD = re.compile(r"^(x\d+), (x\d+), #(0x[0-9a-f]+)$")


def disasm_all(md, data, vaddr):
    """capstone 遇到无效指令会停止；跳过 4 字节后继续，保证覆盖整个段。"""
    off = 0
    data = memoryview(data)
    while off < len(data):
        last = None
        for ins in md.disasm_lite(data[off:], vaddr + off):
            last = ins
            yield ins
        off = (last[0] - vaddr + 4) if last else off
        if last is None or last[0] - vaddr + 4 <= off:
            yield (vaddr + off, 4, "ret", "")  # 无效字节当作函数边界
            off += 4


def main(path, prefix, min_hits=1):
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    chains = defaultdict(Counter)  # global -> Counter(tuple(offsets))
    sites = defaultdict(list)
    for vaddr, data in exec_segments(path):
        regs = {}  # reg -> ("page", value) | ("chain", global, offsets)
        for addr, _, mn, ops in disasm_all(md, data, vaddr):
            if mn in ("ret", "b", "br", "bl", "blr"):
                if mn in ("ret", "b", "br"):
                    regs = {}
                else:  # 调用会破坏 x0-x18
                    regs = {k: v for k, v in regs.items() if int(k[1:]) >= 19}
                continue
            if mn == "adrp":
                m = ADRP.match(ops)
                if m:
                    regs[m.group(1)] = ("page", int(m.group(2), 16))
                continue
            if mn == "add":
                m = ADD.match(ops)
                if m and regs.get(m.group(2), ("",))[0] == "page":
                    regs[m.group(1)] = ("addr", regs[m.group(2)][1] + int(m.group(3), 16))
                    continue
            if mn == "ldr":
                m = LDR.match(ops)
                if m:
                    dst, base, off = m.group(1), m.group(2), int(m.group(3) or "0", 16)
                    src = regs.get(base)
                    if src and src[0] == "page":
                        regs[dst] = ("chain", src[1] + off, ())
                    elif src and src[0] == "addr":
                        regs[dst] = ("chain", src[1] + off, ())
                    elif src and src[0] == "chain":
                        g, offs = src[1], src[2] + (off,)
                        regs[dst] = ("chain", g, offs)
                        if offs[: len(prefix)] == prefix and len(offs) >= len(prefix):
                            chains[g][offs] += 1
                            if len(sites[g]) < 5:
                                sites[g].append(addr)
                    else:
                        regs.pop(dst, None)
                    continue
            # 其他写寄存器的指令：粗略地让目的寄存器失效
            d = ops.split(",", 1)[0].strip()
            if d.startswith("x") or d.startswith("w"):
                regs.pop("x" + d[1:], None)

    ranked = sorted(chains.items(), key=lambda kv: -sum(kv[1].values()))
    for g, cnt in ranked:
        total = sum(cnt.values())
        if total < min_hits:
            continue
        print(f"全局 {g:#x}: {total} 次  示例 {[hex(a) for a in sites[g]]}")
        for offs, c in cnt.most_common(8):
            print(f"    {' -> '.join(hex(o) for o in offs)}  x{c}")


if __name__ == "__main__":
    pre = tuple(int(x, 16) for x in sys.argv[2].split(","))
    main(sys.argv[1], pre, int(sys.argv[3]) if len(sys.argv) > 3 else 1)
