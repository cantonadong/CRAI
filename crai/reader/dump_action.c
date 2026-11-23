// 只读：dump 双方 player+0x258/+0x270 指向的动作对象的前 0x60 字节，
// 对每个 8 字节值，若是指针则尝试读 +0x40 / +0x10->+0x40 处的 i32（找卡牌 ID）。
// 用法: dump_action <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char path[64], line[512]; uint64_t base = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f)) {
        uint64_t a, off;
        if (strstr(line, "libg.so") && sscanf(line, "%" SCNx64 "-%*" SCNx64 " %*s %" SCNx64, &a, &off) == 2 && off == 0) { base = a; break; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    uint64_t roster = rp(rp(rp(base + 0x1b4b928) + 0x2a8) + 0x310);
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(roster + 0xe0 + 8 * s);
        for (int which = 0; which < 2; which++) {
            int fo = which ? 0x270 : 0x258;
            uint64_t act = rp(k + fo);
            printf("p%d +0x%x -> 0x%" PRIx64 "\n", s, fo, act);
            uint8_t b[0x60];
            if (!act || !rd(act, b, sizeof b)) continue;
            for (int o = 0; o < 0x60; o += 8) {
                uint64_t v = *(uint64_t *)(b + o);
                int32_t lo = (int32_t)v, hi = (int32_t)(v >> 32), id40 = 0, id1040 = 0;
                if (v > 0x10000 && !(v & 7)) { rd(v + 0x40, &id40, 4); uint64_t w = rp(v + 0x10); if (w) rd(w + 0x40, &id1040, 4); }
                printf("   +0x%02x: %016" PRIx64 "  i32=(%d,%d)  [+0x40]=%d [+0x10>+0x40]=%d\n", o, v, lo, hi, id40, id1040);
            }
        }
    }
    return 0;
}
