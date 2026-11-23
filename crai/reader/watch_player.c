// 只读监视若干 player 地址：每 250ms 输出圣水、手牌、下一张队列。
// 用法: watch_player <pid> <秒数> <addr1> [addr2 ...]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static int fd;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: %s pid secs addr...\n", argv[0]); return 1; }
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    if (fd < 0) { perror("mem"); return 1; }
    int iters = atoi(argv[2]) * 4;
    for (int it = 0; it < iters; it++) {
        for (int k = 3; k < argc; k++) {
            uint64_t p = strtoull(argv[k], NULL, 16);
            uint8_t o[0x300];
            if (!rd(p, o, sizeof o)) { printf("%s read fail\n", argv[k]); continue; }
            int32_t mana = *(int32_t *)(o + 0x2f8);
            uint64_t hp = *(uint64_t *)(o + 0x210), qp = *(uint64_t *)(o + 0x220);
            int32_t qc = *(int32_t *)(o + 0x22c);
            int32_t h[4] = {-9, -9, -9, -9}, q[4] = {-9, -9, -9, -9};
            rd(hp, h, 16);
            if (qc > 0 && qc <= 8) rd(qp, q, 16);
            printf("t=%5.2f p%d mana=%5.2f hand=[%d,%d,%d,%d] next=[%d,%d,%d,%d]  ",
                   it / 4.0, k - 3, mana / 10000.0, h[0], h[1], h[2], h[3], q[0], q[1], q[2], q[3]);
        }
        printf("\n");
        fflush(stdout);
        usleep(250000);
    }
    return 0;
}
