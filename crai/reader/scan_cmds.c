// 只读：寻找回放出牌指令数组的候选。
// 指令里的落点坐标在格子中心：x%1000∈{499,500,501} 且 0<x<18000，y 同理 0<y<32000（int32 相邻，x 在前）。
// 找出这样的 (x,y) 对按固定步长 stride（8..128 字节）连续出现 >= MINRUN 次的区段，打印起点、步长、条数和前几条内容。
// 用法: scan_cmds <pid> [MINRUN=12]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int is_center(int32_t v, int32_t max) {
    if (v <= 0 || v >= max) return 0;
    int r = v % 1000;
    return r >= 499 && r <= 501;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    int minrun = argc > 2 ? atoi(argv[2]) : 12;
    char path[64], line[512];
    uint64_t lo[8192], hi[8192]; int n = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f) && n < 8192) {
        uint64_t a, b; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %*s %*s %*s %255[^\n]", &a, &b, perms, name) < 3) continue;
        char *s = name; while (*s == ' ') s++;
        int anon = !*s || !strncmp(s, "[anon:", 6) || !strcmp(s, "[heap]");
        if (perms[0] == 'r' && perms[1] == 'w' && anon && !strstr(s, "stack") && b - a <= (512ull << 20)) { lo[n] = a; hi[n] = b; n++; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDONLY);
    const size_t CH = 4 << 20;
    int32_t *buf = malloc(CH + 4096);
    int found = 0;
    for (int r = 0; r < n && found < 200; r++)
        for (uint64_t b = lo[r]; b < hi[r] && found < 200; b += CH) {
            size_t len = hi[r] - b < CH + 4096 ? hi[r] - b : CH + 4096;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            size_t nw = len / 4, lim = (len > CH ? CH : len) / 4;
            for (size_t i = 0; i + 1 < lim && found < 200; i++) {
                if (!is_center(buf[i], 18000) || !is_center(buf[i + 1], 32000)) continue;
                for (int st = 2; st <= 32; st++) {   // 步长（以 int32 为单位）：8..128 字节
                    int run = 1;
                    size_t j = i + st;
                    while (j + 1 < nw && is_center(buf[j], 18000) && is_center(buf[j + 1], 32000)) { run++; j += st; }
                    // 要求是区段起点（前一条不满足），避免重复报告
                    int is_start = i < (size_t)st || !(is_center(buf[i - st], 18000) && is_center(buf[i - st + 1], 32000));
                    // 落点应分散：不同 (x,y) 的数量至少为条数的 1/3
                    int distinct = 0;
                    if (run >= minrun && is_start) {
                        int32_t seen[512][2]; int ns = 0;
                        for (int k = 0; k < run && k < 512; k++) {
                            size_t q = i + (size_t)k * st;
                            int dup = 0;
                            for (int z = 0; z < ns; z++) if (seen[z][0] == buf[q] && seen[z][1] == buf[q + 1]) { dup = 1; break; }
                            if (!dup && ns < 512) { seen[ns][0] = buf[q]; seen[ns][1] = buf[q + 1]; ns++; }
                        }
                        distinct = ns;
                    }
                    if (run >= minrun && is_start && distinct * 3 >= run) {
                        printf("addr=0x%" PRIx64 " stride=%dB count=%d first:", b + i * 4, st * 4, run);
                        for (int k = 0; k < 5; k++) {
                            size_t q = i + (size_t)k * st;
                            printf(" (%d,%d)", buf[q], buf[q + 1]);
                        }
                        printf(" | record0 i32:");
                        for (int k = -4; k < st && k < 12; k++) printf(" %d", (i + k < nw) ? buf[i + k] : 0);
                        printf("\n");
                        found++;
                        break;
                    }
                }
            }
        }
    fprintf(stderr, "found %d\n", found);
    return 0;
}
