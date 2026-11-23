// 写内存（仅用于本机回放实验）：在地址 A 写入一个 int32，写前写后各读一次确认。
// 用法: poke <pid> <addr hex> <int32 value>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 4) return 1;
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    uint64_t a = strtoull(argv[2], NULL, 16);
    int32_t v = (int32_t)strtol(argv[3], NULL, 10), before = 0, after = 0;
    pread64(fd, &before, 4, (off64_t)a);
    if (pwrite64(fd, &v, 4, (off64_t)a) != 4) { perror("pwrite"); return 1; }
    pread64(fd, &after, 4, (off64_t)a);
    printf("0x%" PRIx64 ": %d -> %d (读回 %d)\n", a, before, v, after);
    return after == v ? 0 : 2;
}
