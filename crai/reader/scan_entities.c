// 只读扫描场上实体候选（字段参考旧 ARM 版笔记，待本构建验证）：
//   +0x30 kind i32 [10,20]，+0x78 side 0/1，+0x7c/+0x80 x/y（x<18000, y<32000）
//   +0x84/+0x88 次坐标与主坐标相差 <=200，+0xac card id（-1 或 26/27/28/159xxxxxx 段）
// 用法: scan_entities <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char path[64], line[512];
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    if (!f) return 1;
    uint64_t lo[8192], hi[8192]; int n = 0;
    while (fgets(line, sizeof line, f) && n < 8192) {
        uint64_t a, b; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %*s %*s %*s %255[^\n]", &a, &b, perms, name) < 3) continue;
        char *s = name; while (*s == ' ') s++;
        int anon = !*s || !strncmp(s, "[anon:", 6) || !strcmp(s, "[heap]");
        if (perms[0] == 'r' && perms[1] == 'w' && anon && !strstr(s, "stack") && b - a <= (512ull << 20)) { lo[n] = a; hi[n] = b; n++; }
    }
    fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 1;
    const size_t CH = 4 << 20;
    uint8_t *buf = malloc(CH);
    int found = 0;
    for (int r = 0; r < n; r++)
        for (uint64_t base = lo[r]; base < hi[r]; base += CH) {
            size_t len = hi[r] - base < CH ? hi[r] - base : CH;
            if (pread64(fd, buf, len, (off64_t)base) != (ssize_t)len) continue;
            for (size_t o = 0; o + 0x130 <= len; o += 8) {
                int32_t *v = (int32_t *)(buf + o);
                int32_t kind = v[0x30 / 4], side = v[0x78 / 4], x = v[0x7c / 4], y = v[0x80 / 4];
                int32_t x2 = v[0x84 / 4], y2 = v[0x88 / 4], cid = v[0xac / 4], lvl = v[0x120 / 4];
                if (kind < 10 || kind > 20 || (side != 0 && side != 1)) continue;
                if (x <= 0 || x >= 18000 || y <= 0 || y >= 32000) continue;
                if (abs(x - x2) > 200 || abs(y - y2) > 200) continue;
                if (lvl < 0 || lvl > 16) continue;
                if (cid != -1 && !(cid >= 26000000 && cid < 29000000) && !(cid >= 159000000 && cid < 160000000)) continue;
                if (*(uint64_t *)(buf + o) < 0x10000) continue;  // 首字段应为 vtable 指针
                printf("ent=0x%" PRIx64 " vt=0x%" PRIx64 " kind=%d side=%d pos=(%d,%d) card=%d lvl=%d\n",
                       base + o, *(uint64_t *)(buf + o), kind, side, x, y, cid, lvl + 1);
                if (++found >= 300) goto done;
            }
        }
done:
    fprintf(stderr, "%d candidates\n", found);
    return 0;
}
