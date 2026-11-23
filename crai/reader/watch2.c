// 只读监视（通过静态路径定位双方）：手牌/队列/reveal 列表(+0x288 i32[8])/当前动作(+0x258)/动作序号(+0x318)
// 只在状态变化时输出一行。用法: watch2 <pid> <秒数>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int fd;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    char path[64], line[512]; uint64_t base = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f)) {
        uint64_t a, off;
        if (strstr(line, "libg.so") && sscanf(line, "%" SCNx64 "-%*" SCNx64 " %*s %" SCNx64, &a, &off) == 2 && off == 0) { base = a; break; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    if (fd < 0 || !base) return 1;
    char last[2][512] = {"", ""};
    int iters = atoi(argv[2]) * 100;
    for (int it = 0; it < iters; it++) {
        uint64_t roster = rp(rp(rp(base + 0x1b4b928) + 0x2a8) + 0x310);
        for (int s = 0; s < 2; s++) {
            uint64_t k = rp(roster + 0xe0 + 8 * s);
            int32_t h[4] = {0}, q[4] = {0}, rev[8] = {0}, mana = 0;
            uint32_t meta = 0; uint64_t seq = 0;
            rd(rp(k + 0x210), h, 16); rd(rp(k + 0x220), q, 16); rd(k + 0x288, rev, 32); rd(k + 0x2f8, &mana, 4);
            uint64_t act = rp(k + 0x258); rd(k + 0x318, &seq, 8);
            int32_t aid = -1;
            if (act) { uint64_t prim = rp(act); if (prim) rd(prim + 0x40, &aid, 4); rd(act + 0x10, &meta, 4); }
            char cur[512];
            snprintf(cur, sizeof cur, "hand=[%d,%d,%d,%d] next=[%d,%d,%d,%d] reveal=[%d,%d,%d,%d,%d,%d,%d,%d] act_seq=%" PRIu64 " act_card=%d act_deckidx=%d act_type=%u",
                     h[0], h[1], h[2], h[3], q[0], q[1], q[2], q[3], rev[0], rev[1], rev[2], rev[3], rev[4], rev[5], rev[6], rev[7],
                     seq, aid, (int)((meta >> 22) & 0x3f) - 1, meta & 0xf);
            if (strcmp(cur, last[s])) {
                printf("t=%6.2f p%d mana=%.1f %s\n", it / 100.0, s, mana / 10000.0, cur);
                fflush(stdout);
                strcpy(last[s], cur);
            }
        }
        usleep(10000);
    }
    return 0;
}
