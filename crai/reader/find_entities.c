// 只读：从 root(a)/b/roster 出发（深度<=3，字段<=0x400），寻找存放实体指针的容器：
//   形如 {data 指针, count} 且 data[0..count) 中大部分指向实体（vtable = unit/king）。
// 同时打印 HP 验证：对找到的实体读取 HP component。
// 用法: find_entities <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static uint64_t base, unit_vt, king_vt;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }
static int okp(uint64_t p) { return p > 0x10000 && !(p & 7) && p < 0x800000000000ull; }
static int is_ent(uint64_t p) { if (!okp(p)) return 0; uint64_t v = rp(p); return v == unit_vt || v == king_vt; }

static void hp_of(uint64_t e, int32_t *hp, int32_t *mx) {
    uint8_t k = 0; int32_t s = 0; *hp = *mx = -1;
    rd(e + 0x30, &k, 1); rd(e + 0x24, &s, 4);
    if (!(k & 4) || s < 3) return;
    uint64_t c = rp(rp(e + 0x18) + 0x10);
    if (okp(c)) { rd(c + 0x10, hp, 4); rd(c + 0x14, mx, 4); }
}

#define MAXSEEN 20000
static uint64_t seen[MAXSEEN]; static int nseen;
static int visited(uint64_t p) { for (int i = 0; i < nseen; i++) if (seen[i] == p) return 1; if (nseen < MAXSEEN) seen[nseen++] = p; return 0; }

static int reported;
static void scan(uint64_t obj, int depth, const char *trail) {
    if (visited(obj)) return;
    uint64_t q[0x80];
    if (!rd(obj, q, sizeof q)) return;
    for (int i = 0; i < 0x80 - 1; i++) {
        uint64_t data = q[i];
        if (!okp(data)) continue;
        int32_t c1 = (int32_t)q[i + 1], c2 = (int32_t)(q[i + 1] >> 32);
        int cnt = (c1 >= 3 && c1 <= 4096) ? c1 : (c2 >= 3 && c2 <= 4096) ? c2 : 0;
        if (!cnt) continue;
        int n = cnt > 256 ? 256 : cnt, ents = 0;
        uint64_t arr[256];
        if (!rd(data, arr, 8 * n)) continue;
        for (int j = 0; j < n; j++) ents += is_ent(arr[j]);
        if (ents >= 3 && ents * 2 >= n && reported < 60) {
            reported++;
            printf("%s+0x%x: data, count(%d|%d) -> %d/%d entities\n", trail, i * 8, c1, c2, ents, n);
            int shown = 0;
            for (int j = 0; j < n && shown < 12; j++) {
                uint64_t e = arr[j];
                if (!is_ent(e)) continue;
                int32_t kind, side, x, y, cid, hp, mx;
                rd(e + 0x30, &kind, 4); rd(e + 0x78, &side, 4); rd(e + 0x7c, &x, 4); rd(e + 0x80, &y, 4); rd(e + 0xac, &cid, 4);
                hp_of(e, &hp, &mx);
                printf("    [%d] kind=%d side=%d pos=(%d,%d) card=%d hp=%d/%d\n", j, kind, side, x, y, cid, hp, mx);
                shown++;
            }
        }
    }
    if (depth >= 3) return;
    for (int i = 0; i < 0x80; i++) {
        uint64_t p = q[i];
        if (!okp(p) || (p >= base && p < base + 0x2000000) || is_ent(p)) continue;
        char t[160];
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
    unit_vt = base + 0x1a4fbb8; king_vt = base + 0x1a50840;
    uint64_t a = rp(base + 0x1b4b928), b = rp(a + 0x2a8), roster = rp(b + 0x310);
    if (argc > 2) roster = strtoull(argv[2], NULL, 16);  // 直接指定 roster 地址
    printf("a=0x%" PRIx64 " b=0x%" PRIx64 " roster=0x%" PRIx64 "\n", a, b, roster);
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(roster + 0xe0 + 8 * s);
        int32_t hp, mx, side; rd(k + 0x78, &side, 4); hp_of(k, &hp, &mx);
        printf("king[%d] side=%d hp=%d/%d\n", s, side, hp, mx);
    }
    scan(roster, 0, "roster");
    for (int s = 0; s < 2; s++) {
        char t[32]; snprintf(t, sizeof t, "king%d", s);
        scan(rp(roster + 0xe0 + 8 * s), 1, t);
    }
    return 0;
}
