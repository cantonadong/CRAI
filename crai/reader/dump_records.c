// 只读：从地址 A 开始，按 stride 字节打印 N 条记录（每条以 i32 显示，并标注疑似指针）。
// 用法: dump_records <pid> <addr hex> <stride> <N> [起始偏移条数，可为负]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 5) return 1;
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDONLY);
    uint64_t a = strtoull(argv[2], NULL, 16);
    int stride = atoi(argv[3]), n = atoi(argv[4]), off = argc > 5 ? atoi(argv[5]) : 0;
    for (int k = off; k < off + n; k++) {
        uint64_t p = a + (int64_t)k * stride;
        int32_t v[64];
        if (pread64(fd, v, stride, (off64_t)p) != stride) { printf("#%d read fail\n", k); continue; }
        printf("#%3d @%" PRIx64 ":", k, p);
        for (int i = 0; i < stride / 4; i++) {
            if (i + 1 < stride / 4 && v[i + 1] == 30264 && (i % 2 == 0)) { printf(" [ptr %08x]", (uint32_t)v[i]); i++; continue; }
            printf(" %d", v[i]);
        }
        printf("\n");
    }
    return 0;
}
