// 只读：先用 vtable 指纹找出场上所有实体，再找出所有"指向实体的指针"所在位置，
// 按"连续存放实体指针的数组"聚类，打印数组起点、长度，以及持有该数组指针的位置。
// 用法: whoref <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint64_t lo[8192], hi[8192]; static int n;
static uint64_t ents[4096]; static int nents;
static int cmpu(const void *a, const void *b) { uint64_t x = *(uint64_t *)a, y = *(uint64_t *)b; return x < y ? -1 : x > y; }
static int is_ent(uint64_t v) {
    int l = 0, h = nents;
    while (l < h) { int m = (l + h) / 2; if (ents[m] < v) l = m + 1; else h = m; }
    return l < nents && ents[l] == v;
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
    // pass 1: 实体
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH ? hi[r] - b : CH;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            for (size_t i = 0; i < len / 8; i++)
                if ((buf[i] == uvt || buf[i] == kvt) && nents < 4096) {
                    int32_t side = -1; pread64(fd, &side, 4, (off64_t)(b + i * 8 + 0x78));
                    if (side == 0 || side == 1) ents[nents++] = b + i * 8;
                }
        }
    qsort(ents, nents, 8, cmpu);
    fprintf(stderr, "%d entities\n", nents);
    // pass 2: 连续实体指针数组（长度>=4）
    int arrays = 0;
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH ? hi[r] - b : CH;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            size_t i = 0;
            while (i < len / 8) {
                if (!is_ent(buf[i])) { i++; continue; }
                size_t j = i; int gaps = 0;
                while (j < len / 8 && (is_ent(buf[j]) || (buf[j] == 0 && gaps++ < 2))) j++;
                int cnt = 0; for (size_t k = i; k < j; k++) cnt += is_ent(buf[k]);
                if (cnt >= 4 && arrays < 40) {
                    arrays++;
                    printf("array @0x%" PRIx64 " slots=%zu ents=%d%s\n", b + i * 8, j - i, cnt, lo[r] >= base && lo[r] < base + 0x2000000 ? " [libg]" : "");
                }
                i = j + 1;
            }
        }
    return 0;
}
