// 只读：寻找"实体列表"容器。
// 1) vtable 指纹扫出当前全部实体集合 E
// 2) 扫描可写内存中由 E 中指针组成的连续数组（允许 0 间隔），覆盖率 >= 70% |E| 的输出
// 3) 对每个数组起点 A，找出值为 A（或 A 前 0x40 内）的指针位置 H（容器持有者），并打印 H 后 16 字节（count/cap 猜测）
// 输出一行一个候选: arr=<A> len=<slots> hit=<n>/<|E|> holder=<H> next=[i32 x4]
// 用法: entlist <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint64_t lo[8192], hi[8192]; static int n;
static uint64_t ents[4096]; static int ne;
static int cmpu(const void *a, const void *b) { uint64_t x = *(uint64_t *)a, y = *(uint64_t *)b; return x < y ? -1 : x > y; }
static int is_ent(uint64_t v) {
    int l = 0, h = ne;
    while (l < h) { int m = (l + h) / 2; if (ents[m] < v) l = m + 1; else h = m; }
    return l < ne && ents[l] == v;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char path[64], line[512]; uint64_t base = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f) && n < 8192) {
        uint64_t a, b, off; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %" SCNx64 " %*s %*s %255[^\n]", &a, &b, perms, &off, name) < 4) continue;
        char *s = name; while (*s == ' ') s++;
        if (strstr(s, "libg.so") && off == 0 && !base) base = a;
        int anon = !*s || !strncmp(s, "[anon:", 6) || !strcmp(s, "[heap]");
        if (perms[0] == 'r' && perms[1] == 'w' && ((anon && !strstr(s, "stack") && b - a <= (512ull << 20)) || strstr(s, "libg.so"))) { lo[n] = a; hi[n] = b; n++; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDONLY);
    uint64_t uvt = base + 0x1a4fbb8, kvt = base + 0x1a50840;
    const size_t CH = 4 << 20;
    uint64_t *buf = malloc(CH);
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH ? hi[r] - b : CH;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            for (size_t i = 0; i < len / 8; i++)
                if ((buf[i] == uvt || buf[i] == kvt) && ne < 4096) {
                    int32_t side = -1, x = -1;
                    pread64(fd, &side, 4, (off64_t)(b + i * 8 + 0x78));
                    pread64(fd, &x, 4, (off64_t)(b + i * 8 + 0x7c));
                    if ((side == 0 || side == 1) && x >= 0 && x <= 18000) ents[ne++] = b + i * 8;
                }
        }
    qsort(ents, ne, 8, cmpu);
    fprintf(stderr, "entities=%d\n", ne);
    uint64_t arrs[256]; int na = 0;
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH ? hi[r] - b : CH;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            size_t i = 0;
            while (i < len / 8) {
                if (!is_ent(buf[i])) { i++; continue; }
                size_t j = i; int gaps = 0, hit = 0;
                while (j < len / 8 && (is_ent(buf[j]) || (buf[j] == 0 && gaps++ < 4))) { hit += is_ent(buf[j]); j++; }
                if (hit * 10 >= ne * 7 && na < 256) {
                    arrs[na++] = b + i * 8;
                    printf("arr=0x%" PRIx64 " len=%zu hit=%d/%d\n", b + i * 8, j - i, hit, ne);
                }
                i = j + 1;
            }
        }
    // 持有者
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH ? hi[r] - b : CH;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            for (size_t i = 0; i + 2 < len / 8; i++)
                for (int k = 0; k < na; k++)
                    if (buf[i] <= arrs[k] && arrs[k] - buf[i] <= 0x40) {
                        int32_t *c = (int32_t *)&buf[i + 1];
                        printf("holder=0x%" PRIx64 " -> arr=0x%" PRIx64 " (-0x%" PRIx64 ") next=[%d,%d,%d,%d]%s\n",
                               b + i * 8, arrs[k], arrs[k] - buf[i], c[0], c[1], c[2], c[3],
                               lo[r] >= base && lo[r] < base + 0x2000000 ? " [libg]" : "");
                    }
        }
    return 0;
}
