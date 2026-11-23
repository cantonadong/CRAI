// 只读探针：
// 1) 按静态路径 libg+0x1b4b928 -> +0x2a8 -> +0x310 -> +0xe0/+0xe8 取双方国王塔，打印圣水/手牌/队列
// 2) 从每个国王塔出发做深度<=3 的指针搜索，找值为卡牌 ID（26/27/28xxxxxx、159xxxxxx）的字段，打印路径
// 用法: probe <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static uint64_t libg_base;
static uint64_t rlo[8192], rhi[8192]; static int nr;

static int ok_ptr(uint64_t p) {
    if (p < 0x10000 || (p & 7)) return 0;
    for (int i = 0; i < nr; i++) if (p >= rlo[i] && p + 8 <= rhi[i]) return 1;
    return 0;
}
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }
static int is_card(int32_t v) {
    return (v >= 26000000 && v < 26000400) || (v >= 27000000 && v < 27000100) ||
           (v >= 28000000 && v < 28000100) || (v >= 159000000 && v < 159000100);
}

static int hits;
static void search(uint64_t p, int depth, char *trail) {
    uint8_t buf[0x300];
    if (!rd(p, buf, sizeof buf)) return;
    for (int o = 0; o < 0x300; o += 4) {
        int32_t v = *(int32_t *)(buf + o);
        if (is_card(v) && depth > 0 && hits < 400) {  // depth>0: 不报国王塔自身字段
            printf("  %s+0x%x = %d\n", trail, o, v);
            hits++;
        }
    }
    if (depth >= 3) return;
    for (int o = 0; o < 0x300; o += 8) {
        uint64_t q = *(uint64_t *)(buf + o);
        if (!ok_ptr(q)) continue;
        if (q >= libg_base && q < libg_base + 0x2000000) continue;  // 跳过 vtable/静态数据
        char t[256];
        snprintf(t, sizeof t, "%s+0x%x>", trail, o);
        search(q, depth + 1, t);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char path[64], line[512];
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f) && nr < 8192) {
        uint64_t a, b, off; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %" SCNx64 " %*s %*s %255[^\n]", &a, &b, perms, &off, name) < 4) continue;
        if (strstr(name, "libg.so") && off == 0 && !libg_base) libg_base = a;
        if (perms[0] == 'r') { rlo[nr] = a; rhi[nr] = b; nr++; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    if (fd < 0 || !libg_base) return 1;

    uint64_t a = rp(libg_base + 0x1b4b928), b = ok_ptr(a) ? rp(a + 0x2a8) : 0, c = ok_ptr(b) ? rp(b + 0x310) : 0;
    printf("a=0x%" PRIx64 " b=0x%" PRIx64 " roster=0x%" PRIx64 "\n", a, b, c);
    if (!ok_ptr(c)) return 1;
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(c + 0xe0 + 8 * s);
        int32_t side = 0, mana = 0, h[4] = {0}, q[4] = {0}, lvl = 0;
        rd(k + 0x78, &side, 4); rd(k + 0x2f8, &mana, 4); rd(k + 0x120, &lvl, 4);
        rd(rp(k + 0x210), h, 16); rd(rp(k + 0x220), q, 16);
        printf("player[%d] king=0x%" PRIx64 " vt_ok=%d side=%d lvl=%d mana=%.2f hand=[%d,%d,%d,%d] next=[%d,%d,%d,%d]\n", s, k,
               rp(k) == libg_base + 0x1a50840, side, lvl + 1, mana / 10000.0, h[0], h[1], h[2], h[3], q[0], q[1], q[2], q[3]);
    }
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(c + 0xe0 + 8 * s);
        printf("== player[%d] 卡牌 ID 搜索\n", s);
        hits = 0;
        search(k, 0, "king");
    }
    return 0;
}
