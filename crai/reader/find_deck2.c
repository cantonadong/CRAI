// 只读：在 roster 的 0x0..0x200 字段里找 "卡组容器"：container+0x20 -> 元素指针数组，count 在 +0x28/+0x2c，
// 元素 e 满足 card(e) 或 card(*(e+0x10))（card: vtable=libg+0x1a3bcf8，ID 在 +0x40）。
// 用法: find_deck2 <pid>
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
static int card_id(uint64_t p) {
    int32_t id = 0;
    if (p < 0x10000 || (p & 7)) return 0;
    if (rp(p) == card_vt) { rd(p + 0x40, &id, 4); return id; }
    uint64_t w = rp(p + 0x10);
    if (w > 0x10000 && !(w & 7) && rp(w) == card_vt) { rd(w + 0x40, &id, 4); return id; }
    return 0;
}

static void try_container(const char *name, uint64_t c) {
    // 容器布局猜测: 数据指针在 +dp，数量在 +dp+8 或 +dp+0xc
    for (int dp = 0; dp <= 0x30; dp += 8) {
        uint64_t data = rp(c + dp);
        if (data < 0x10000 || (data & 7)) continue;
        int32_t n1 = 0, n2 = 0; rd(c + dp + 8, &n1, 4); rd(c + dp + 0xc, &n2, 4);
        int ids[8], ok = 1;
        for (int j = 0; j < 8 && ok; j++) ok = (ids[j] = card_id(rp(data + 8 * j))) != 0;
        if (!ok) continue;
        printf("%s data@+0x%x cnt=(%d,%d): [", name, dp, n1, n2);
        for (int j = 0; j < 8; j++) printf("%d%s", ids[j], j < 7 ? "," : "]\n");
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
    printf("roster=0x%" PRIx64 "\n", roster);
    char nm[64];
    for (int o = 0; o < 0x200; o += 8) {
        uint64_t p = rp(roster + o);
        if (p < 0x10000 || (p & 7)) continue;
        snprintf(nm, sizeof nm, "roster+0x%x>", o);
        try_container(nm, p);
        snprintf(nm, sizeof nm, "roster+0x%x(inline)", o);
    }
    for (int o = 0; o < 0x200; o += 8) { snprintf(nm, sizeof nm, "roster@+0x%x", o); try_container(nm, roster + o); }
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(roster + 0xe0 + 8 * s);
        for (int o = 0; o < 0x400; o += 8) {
            uint64_t p = rp(k + o);
            if (p < 0x10000 || (p & 7) || (p >= base && p < base + 0x2000000)) continue;
            snprintf(nm, sizeof nm, "p%d+0x%x>", s, o); try_container(nm, p);
        }
        for (int o = 0; o < 0x400; o += 8) { snprintf(nm, sizeof nm, "p%d@+0x%x", s, o); try_container(nm, k + o); }
    }
    return 0;
}
