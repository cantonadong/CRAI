// 只读：对 a / b / roster 三个对象各取 0x800 字节，间隔 1 秒采两次，
// 报告"像时钟"的字段：i32 增量在 [15,25]（tick，约 20/s）或 [900,1100]（毫秒）；f32 增量在 [0.9,1.1]（秒）。
// 用法: diff_fields <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char path[64], line[512]; uint64_t base = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f)) {
        uint64_t x, off;
        if (strstr(line, "libg.so") && sscanf(line, "%" SCNx64 "-%*" SCNx64 " %*s %" SCNx64, &x, &off) == 2 && off == 0) { base = x; break; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    uint64_t a = rp(base + 0x1b4b928), b = rp(a + 0x2a8), r = rp(b + 0x310);
    uint64_t objs[3] = {a, b, r}; const char *nm[3] = {"a", "b", "roster"};
    static uint8_t s1[3][0x800], s2[3][0x800];
    for (int i = 0; i < 3; i++) rd(objs[i], s1[i], 0x800);
    usleep(1000000);
    for (int i = 0; i < 3; i++) rd(objs[i], s2[i], 0x800);
    for (int i = 0; i < 3; i++)
        for (int o = 0; o < 0x800; o += 4) {
            int32_t v1 = *(int32_t *)(s1[i] + o), v2 = *(int32_t *)(s2[i] + o);
            float f1 = *(float *)(s1[i] + o), f2 = *(float *)(s2[i] + o);
            int d = v2 - v1;
            if ((d >= 15 && d <= 25) || (d >= 900 && d <= 1100))
                printf("%s+0x%x i32 %d -> %d (d=%d)\n", nm[i], o, v1, v2, d);
            else if (isfinite(f1) && isfinite(f2) && fabsf(f1) < 1e6f && f2 - f1 >= 0.9f && f2 - f1 <= 1.1f)
                printf("%s+0x%x f32 %.3f -> %.3f\n", nm[i], o, f1, f2);
        }
    return 0;
}
