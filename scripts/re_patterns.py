"""在 ARM64 libg.so 中按指令形态搜索已知结构的访问函数（只读静态分析）。

用法: python -I scripts/re_patterns.py apk/libs/arm64-v8a/libg.so
"""
import struct
import sys

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
from elftools.elf.elffile import ELFFile

RET = 0xD65F03C0


def exec_segments(path):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        segs = []
        for s in elf.iter_segments():
            if s["p_type"] == "PT_LOAD" and s["p_flags"] & 1:  # PF_X
                f.seek(s["p_offset"])
                segs.append((s["p_vaddr"], f.read(s["p_filesz"])))
        return segs


def words(data):
    return struct.unpack_from(f"<{len(data) // 4}I", data)


def disasm(md, data, vaddr, start, n):
    off = start - vaddr
    return [f"  {i.address:#x}: {i.mnemonic} {i.op_str}" for i in md.disasm(data[off:off + 4 * n], start)]


def main(path):
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    for vaddr, data in exec_segments(path):
        w = words(data)
        print(f"== 可执行段 {vaddr:#x} 大小 {len(data):#x}")

        # 1) 手牌槽 -> deck index:  ldr xA,[x0,#imm]; ldr w0,[xA,w1,sxtw #2]; ret
        print("-- 手牌 getter 候选 (model+imm -> int[slot])")
        for i in range(len(w) - 2):
            a, b, c = w[i], w[i + 1], w[i + 2]
            if c != RET or (a & 0xFFC003E0) != 0xF9400000:
                continue
            rt = a & 0x1F
            # ldr w0,[x<rt>,w1,sxtw #2] = 0xB861D800 | rt<<5
            if b == (0xB861D800 | (rt << 5)):
                imm = ((a >> 10) & 0xFFF) * 8
                print(f"  {vaddr + 4 * i:#x}: model+{imm:#x}")

        # 2) 队首 helper: ldr w8,[x0,#0x2c] ... ldr x8,[x0,#0x20]; ldr w0,[x8]
        print("-- 队列首项 helper 候选 (count@+0x2c, data@+0x20)")
        for i in range(len(w) - 6):
            if (w[i] & 0xFFC003FF) == 0xB9400008 and ((w[i] >> 10) & 0xFFF) * 4 == 0x2C:
                win = w[i:i + 7]
                if any((x & 0xFFFFFFFF) == 0xF9401008 for x in win):  # ldr x8,[x0,#0x20]
                    print(f"  {vaddr + 4 * i:#x}")
                    print("\n".join(disasm(md, data, vaddr, vaddr + 4 * i, 7)))

        # 3) 圣水 /10000 的魔数 0x68db8bad：movz #0x8bad + movk #0x68db, lsl #16
        print("-- 常数 0x68db8bad (raw mana / 10000) 的使用点")
        hits = 0
        for i in range(len(w) - 1):
            a, b = w[i], w[i + 1]
            if (a & 0xFFFFFFE0) == (0x52800000 | (0x8BAD << 5)) and (b & 0xFFFFFFE0) == (0x72A00000 | (0x68DB << 5)):
                hits += 1
                if hits <= 15:
                    print(f"  {vaddr + 4 * i:#x}")
                    print("\n".join(disasm(md, data, vaddr, vaddr + 4 * (i - 4), 10)))
        print(f"  共 {hits} 处")


if __name__ == "__main__":
    main(sys.argv[1])
