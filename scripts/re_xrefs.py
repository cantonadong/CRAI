"""查找对某个函数地址的 BL 调用点，并反汇编调用点前后若干指令。

用法: python -I scripts/re_xrefs.py <libg.so> <目标地址hex> [前N条] [后N条]
"""
import struct
import sys

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs

sys.path.insert(0, __file__.rsplit("\\", 1)[0].rsplit("/", 1)[0])
from re_patterns import exec_segments  # noqa: E402


def main(path, target, before=6, after=3, limit=40):
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    n = 0
    for vaddr, data in exec_segments(path):
        w = struct.unpack_from(f"<{len(data) // 4}I", data)
        for i, x in enumerate(w):
            if (x & 0xFC000000) != 0x94000000:  # BL
                continue
            imm = x & 0x3FFFFFF
            if imm & 0x2000000:
                imm -= 0x4000000
            pc = vaddr + 4 * i
            if pc + imm * 4 != target:
                continue
            n += 1
            if n > limit:
                continue
            s = max(0, i - before)
            print(f"-- 调用点 {pc:#x}")
            for ins in md.disasm(data[4 * s:4 * (i + after + 1)], vaddr + 4 * s):
                print(f"  {ins.address:#x}: {ins.mnemonic} {ins.op_str}")
    print(f"共 {n} 处调用")


if __name__ == "__main__":
    a = sys.argv
    main(a[1], int(a[2], 16), *(int(v) for v in a[3:5]))
