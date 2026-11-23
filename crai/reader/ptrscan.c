// 只读指针反查：从目标对象（默认：所有国王塔实体）出发，逐层寻找指向它的指针，
// 直到落入 libg.so 的可写数据段，输出 "libg+偏移 -> +off -> ... -> 目标" 的路径。
// 用法: ptrscan <pid> [最大深度=5] [最大字段偏移=0x400]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KING_VT_OFF 0x1a50840ull
#define MAXN 200000

typedef struct { uint64_t lo, hi; int is_libg; } Reg;
typedef struct { uint64_t addr; int parent; uint32_t off; int depth; } Node;

static Reg regs[8192]; static int nregs;
static uint64_t libg_base;
static int fd;
static Node nodes[MAXN]; static int nn;

static int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
    return x < y ? -1 : x > y;
}

static void print_path(int i) {
    // i 是落在 libg 数据段的节点：沿 parent 输出到根（目标）
    printf("libg+0x%" PRIx64, nodes[i].addr - libg_base);
    for (int k = i; nodes[k].parent >= 0; k = nodes[k].parent)
        printf(" -> [+0x%x]", nodes[k].off);
    printf("  => target 0x%" PRIx64 "\n", nodes[i].parent >= 0 ? ({ int k = i; while (nodes[k].parent >= 0) k = nodes[k].parent; nodes[k].addr; }) : nodes[i].addr);
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    int maxdepth = argc > 2 ? atoi(argv[2]) : 5;
    uint64_t maxoff = argc > 3 ? strtoull(argv[3], NULL, 16) : 0x400;
    char path[64], line[512];
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    if (!f) return 1;
    while (fgets(line, sizeof line, f) && nregs < 8192) {
        uint64_t a, b, off; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %" SCNx64 " %*s %*s %255[^\n]", &a, &b, perms, &off, name) < 4) continue;
        char *s = name; while (*s == ' ') s++;
        int libg = strstr(s, "libg.so") != NULL;
        if (libg && off == 0 && !libg_base) libg_base = a;
        int anon = !*s || !strncmp(s, "[anon:", 6) || !strcmp(s, "[heap]");
        if (perms[0] == 'r' && perms[1] == 'w' && ((anon && !strstr(s, "stack") && b - a <= (512ull << 20)) || libg))
            regs[nregs++] = (Reg){a, b, libg};
    }
    fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    if (fd < 0 || !libg_base) { fprintf(stderr, "open/libg fail\n"); return 1; }
    uint64_t king_vt = libg_base + KING_VT_OFF;
    fprintf(stderr, "libg base=0x%" PRIx64 " king vtable=0x%" PRIx64 "\n", libg_base, king_vt);

    const size_t CH = 4 << 20;
    uint64_t *buf = malloc(CH);

    // 第 0 层：找国王塔
    for (int r = 0; r < nregs; r++)
        for (uint64_t base = regs[r].lo; base < regs[r].hi; base += CH) {
            size_t len = regs[r].hi - base < CH ? regs[r].hi - base : CH;
            if (pread64(fd, buf, len, (off64_t)base) != (ssize_t)len) continue;
            for (size_t i = 0; i < len / 8; i++)
                if (buf[i] == king_vt && nn < 16) {
                    int32_t side = 0;
                    pread64(fd, &side, 4, (off64_t)(base + i * 8 + 0x78));
                    fprintf(stderr, "king tower 0x%" PRIx64 " side=%d\n", base + i * 8, side);
                    nodes[nn++] = (Node){base + i * 8, -1, 0, 0};
                }
        }
    if (!nn) { fprintf(stderr, "no king tower found (not in battle?)\n"); return 1; }

    // 模式 roster：目标改为 "+0xe0/+0xe8 依次存放两座国王塔指针" 的 roster 对象
    if (argc > 4 && !strcmp(argv[4], "roster") && nn == 2) {
        uint64_t k0 = nodes[0].addr, k1 = nodes[1].addr;
        int found_r = 0;
        for (int r = 0; r < nregs; r++)
            for (uint64_t base = regs[r].lo; base < regs[r].hi; base += CH) {
                size_t len = regs[r].hi - base < CH ? regs[r].hi - base : CH;
                if (pread64(fd, buf, len, (off64_t)base) != (ssize_t)len) continue;
                for (size_t i = 0; i + 1 < len / 8; i++)
                    if ((buf[i] == k0 && buf[i + 1] == k1) || (buf[i] == k1 && buf[i + 1] == k0)) {
                        uint64_t ro = base + i * 8 - 0xe0;
                        fprintf(stderr, "roster candidate 0x%" PRIx64 " (first=%s)\n", ro, buf[i] == k0 ? "side0" : "side1");
                        if (found_r < 8) nodes[found_r++] = (Node){ro, -1, 0, 0};
                    }
            }
        nn = found_r;
        if (!nn) { fprintf(stderr, "no roster found\n"); return 1; }
    }

    int lvl_start = 0, found = 0;
    for (int d = 1; d <= maxdepth && nn < MAXN; d++) {
        int lvl_end = nn;
        int cnt = lvl_end - lvl_start;
        uint64_t *tg = malloc(sizeof(uint64_t) * 2 * cnt);  // (addr, node index) 按 addr 排序
        for (int i = 0; i < cnt; i++) { tg[2 * i] = nodes[lvl_start + i].addr; tg[2 * i + 1] = lvl_start + i; }
        qsort(tg, cnt, 16, cmp_u64);
        for (int r = 0; r < nregs && nn < MAXN; r++)
            for (uint64_t base = regs[r].lo; base < regs[r].hi && nn < MAXN; base += CH) {
                size_t len = regs[r].hi - base < CH ? regs[r].hi - base : CH;
                if (pread64(fd, buf, len, (off64_t)base) != (ssize_t)len) continue;
                for (size_t i = 0; i < len / 8 && nn < MAXN; i++) {
                    uint64_t v = buf[i];
                    if (v < 0x10000 || (v & 7)) continue;
                    // 找第一个 target >= v，检查 target - v <= maxoff
                    int lo = 0, hi = cnt;
                    while (lo < hi) { int m = (lo + hi) / 2; if (tg[2 * m] < v) lo = m + 1; else hi = m; }
                    for (int k = lo; k < cnt && tg[2 * k] - v <= maxoff; k++) {
                        uint64_t at = base + i * 8;
                        nodes[nn] = (Node){at, (int)tg[2 * k + 1], (uint32_t)(tg[2 * k] - v), d};
                        if (regs[r].is_libg) { print_path(nn); found++; }
                        nn++;
                        if (nn >= MAXN) break;
                    }
                }
            }
        free(tg);
        fprintf(stderr, "depth %d: %d nodes (total %d), %d static paths so far\n", d, nn - lvl_end, nn, found);
        lvl_start = lvl_end;
        if (found >= 50) break;
    }
    return 0;
}
