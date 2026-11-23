// 只读：从双方 player 出发（深度<=2，字段<=0x400），寻找 "连续 8 个指针都指向卡牌对象(vtable=libg+0x1a3bcf8)" 的数组，
// 打印路径和 8 个卡牌 ID。用法: find_deck <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static uint64_t base, card_vt;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }
static int card_of(uint64_t p, int32_t *id) {
    if (p < 0x10000 || (p & 7)) return 0;
    if (rp(p) != card_vt) return 0;
    return rd(p + 0x40, id, 4);
}

static void scan(uint64_t obj, int depth, const char *trail) {
    uint64_t q[0x80 + 8];
    memset(q, 0, sizeof q);
    if (!rd(obj, q, 0x400)) return;
    for (int i = 0; i + 8 <= 0x80; i++) {
        int32_t ids[8]; int ok = 1;
        for (int j = 0; j < 8 && ok; j++) ok = card_of(q[i + j], &ids[j]);
        if (ok) {
            printf("%s+0x%x: [", trail, i * 8);
            for (int j = 0; j < 8; j++) printf("%d%s", ids[j], j < 7 ? "," : "]\n");
        }
    }
    if (depth >= 2) return;
    for (int i = 0; i < 0x80; i++) {
        uint64_t p = q[i];
        if (p < 0x10000 || (p & 7) || (p >= base && p < base + 0x2000000)) continue;
        char t[128];
        snprintf(t, sizeof t, "%s+0x%x>", trail, i * 8);
        scan(p, depth + 1, t);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char path[64], line[512];
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f)) {
        uint64_t a, off;
        if (strstr(line, "libg.so") && sscanf(line, "%" SCNx64 "-%*" SCNx64 " %*s %" SCNx64, &a, &off) == 2 && off == 0) { base = a; break; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    card_vt = base + 0x1a3bcf8;
    uint64_t roster = rp(rp(rp(base + 0x1b4b928) + 0x2a8) + 0x310);
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(roster + 0xe0 + 8 * s);
        printf("== p%d king=0x%" PRIx64 "\n", s, k);
        scan(k, 0, "king");
    }
    return 0;
}
