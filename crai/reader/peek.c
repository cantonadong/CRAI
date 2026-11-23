// 只读：打印 libg+偏移 处的 n 个 qword，以及每个值相对 libg 的偏移（若落在 libg 内）。
// 用法: peek <pid> <libg偏移hex> <n>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 4) return 1;
    char path[64], line[512]; uint64_t base = 0, end = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f)) {
        uint64_t a, b, off;
        if (strstr(line, "libg.so") && sscanf(line, "%" SCNx64 "-%" SCNx64 " %*s %" SCNx64, &a, &b, &off) == 3) {
            if (off == 0 && !base) base = a;
            if (b > end) end = b;
        }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDONLY);
    if (fd < 0 || !base) return 1;
    uint64_t off = strtoull(argv[2], NULL, 16);
    int n = atoi(argv[3]);
    for (int i = 0; i < n; i++) {
        uint64_t v = 0;
        pread64(fd, &v, 8, (off64_t)(base + off + 8 * i));
        if (v >= base && v < end)
            printf("+0x%03x: 0x%016" PRIx64 "  libg+0x%" PRIx64 "\n", 8 * i, v, v - base);
        else
            printf("+0x%03x: 0x%016" PRIx64 "\n", 8 * i, v);
    }
    return 0;
}
