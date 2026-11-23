// 只读：找出所有值落在 [target-0x40, target] 的 8 字节位置（即指向目标或其前方的指针）。
// 对每个位置，打印它附近 +8/+0xc 的 i32（容器 count/capacity 猜测）。
// 用法: findptr <pid> <target1> [target2 ...]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    char path[64], line[512]; uint64_t lo[8192], hi[8192], base = 0; int n = 0;
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
    int nt = argc - 2; uint64_t t[64];
    for (int i = 0; i < nt && i < 64; i++) t[i] = strtoull(argv[2 + i], NULL, 16);
    const size_t CH = 4 << 20;
    uint64_t *buf = malloc(CH + 16);
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH ? hi[r] - b : CH;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            for (size_t i = 0; i < len / 8; i++)
                for (int k = 0; k < nt; k++)
                    if (buf[i] <= t[k] && t[k] - buf[i] <= 0x40) {
                        int32_t c[4] = {0};
                        pread64(fd, c, 16, (off64_t)(b + i * 8 + 8));
                        printf("target 0x%" PRIx64 " <- @0x%" PRIx64 " (val=0x%" PRIx64 ", -0x%" PRIx64 ") next i32: %d %d %d %d%s\n",
                               t[k], b + i * 8, buf[i], t[k] - buf[i], c[0], c[1], c[2], c[3],
                               b >= base && b < base + 0x2000000 ? " [libg]" : "");
                    }
        }
    return 0;
}
