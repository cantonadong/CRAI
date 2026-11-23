// 只读：每 0.5s 按 list = *(*(roster+0x8)+0x40) 读实体列表，统计其中有效实体，并与 vtable 全扫描比较。
// 用法: verify_entlist <pid> <roster hex> [次数=20]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static uint64_t base, uvt, kvt;
static uint64_t lo[8192], hi[8192]; static int n;
static int rd(uint64_t a, void *b, size_t k) { return pread64(fd, b, k, (off64_t)a) == (ssize_t)k; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }
static int cmpu(const void *a, const void *b) { uint64_t x = *(uint64_t *)a, y = *(uint64_t *)b; return x < y ? -1 : x > y; }

static int scan_all(uint64_t *out) {
    static uint64_t buf[(4 << 20) / 8];
    int ne = 0;
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += sizeof buf) {
            size_t len = hi[r] - b < sizeof buf ? hi[r] - b : sizeof buf;
            if (!rd(b, buf, len)) continue;
            for (size_t i = 0; i < len / 8; i++)
                if (buf[i] == uvt || buf[i] == kvt) {
                    int32_t side = -1; rd(b + i * 8 + 0x78, &side, 4);
                    if ((side == 0 || side == 1) && ne < 4096) out[ne++] = b + i * 8;
                }
        }
    qsort(out, ne, 8, cmpu);
    return ne;
}

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    char path[64], line[512];
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f) && n < 8192) {
        uint64_t a, b, off; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %" SCNx64 " %*s %*s %255[^\n]", &a, &b, perms, &off, name) < 4) continue;
        char *s = name; while (*s == ' ') s++;
        if (strstr(s, "libg.so") && off == 0 && !base) base = a;
        if (perms[0] == 'r' && perms[1] == 'w' && strstr(s, "libc_malloc")) { lo[n] = a; hi[n] = b; n++; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    uvt = base + 0x1a4fbb8; kvt = base + 0x1a50840;
    uint64_t roster = strtoull(argv[2], NULL, 16);
    int iters = argc > 3 ? atoi(argv[3]) : 20;
    static uint64_t all[4096];
    for (int it = 0; it < iters; it++) {
        uint64_t L = rp(rp(roster + 0x8) + 0x40);
        uint64_t data = rp(L + 0x8);
        int32_t cap = 0, cnt = 0; rd(L + 0x10, &cap, 4); rd(L + 0x14, &cnt, 4);
        uint64_t arr[512] = {0};
        int k = cnt > 0 && cnt <= 512 ? cnt : 0, valid = 0, other = 0;
        if (k) rd(data, arr, 8 * k);
        for (int i = 0; i < k; i++) { uint64_t vt = rp(arr[i]); if (vt == uvt || vt == kvt) valid++; else other++; }
        int ne = scan_all(all), inlist = 0;
        for (int j = 0; j < ne; j++) for (int i = 0; i < k; i++) if (arr[i] == all[j]) { inlist++; break; }
        printf("list=0x%" PRIx64 " cap=%d count=%d | 列表中有效实体=%d 其他对象=%d | 全扫描实体=%d 其中在列表中=%d\n",
               L, cap, cnt, valid, other, ne, inlist);
        fflush(stdout);
        usleep(500000);
    }
    return 0;
}
