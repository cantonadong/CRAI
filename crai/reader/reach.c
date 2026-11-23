// 只读：从起点对象出发（深度<=3，每层字段 0..0x800），寻找能到达目标地址 T 的指针路径：
//   *(obj+off) = p，且 T - p 在 [0, 0x800] 内（T 是 p 指向对象内的字段）
// 用法: reach <pid> <起点hex> <目标hex>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static uint64_t T;
static int found;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }

static void go(uint64_t obj, int depth, char *trail) {
    uint64_t q[0x100];
    if (!rd(obj, q, sizeof q)) return;
    for (int i = 0; i < 0x100; i++) {
        uint64_t p = q[i];
        if (p < 0x10000 || (p & 7) || p > 0x800000000000ull) continue;
        if (T >= p && T - p <= 0x800 && found < 40) {
            printf("%s+0x%x -> 对象 0x%" PRIx64 " , 目标在其 +0x%" PRIx64 "\n", trail, i * 8, p, T - p);
            found++;
        }
        if (depth < 3) {
            char t[256];
            snprintf(t, sizeof t, "%s+0x%x>", trail, i * 8);
            go(p, depth + 1, t);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 4) return 1;
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    uint64_t start = strtoull(argv[2], NULL, 16);
    T = strtoull(argv[3], NULL, 16);
    go(start, 1, "roster");
    fprintf(stderr, "found %d\n", found);
    return 0;
}
