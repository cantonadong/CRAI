// 只读：扫描回放出牌指令对象（96 字节）。识别依据：对象首 8 字节 = VT（默认 0x4DED748，观测值），
// 且 +0x28/+0x2C 为合理坐标、+0x38 指向卡牌对象（vtable = libg+0x1a3bcf8，ID 在 +0x40）。
// 输出（按 tick 排序由调用方完成）：addr tick_a tick_b player x y card_id
// 用法: list_cmds <pid> [VT hex]
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    uint64_t VT = argc > 2 ? strtoull(argv[2], NULL, 16) : 0x4DED748ull;
    char path[64], line[512];
    uint64_t lo[8192], hi[8192], base = 0; int n = 0;
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f) && n < 8192) {
        uint64_t a, b, off; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %" SCNx64 " %*s %*s %255[^\n]", &a, &b, perms, &off, name) < 4) continue;
        char *s = name; while (*s == ' ') s++;
        if (strstr(s, "libg.so") && off == 0 && !base) base = a;
        int anon = !*s || !strncmp(s, "[anon:", 6) || !strcmp(s, "[heap]");
        if (perms[0] == 'r' && perms[1] == 'w' && anon && !strstr(s, "stack") && b - a <= (512ull << 20)) { lo[n] = a; hi[n] = b; n++; }
    }
    if (f) fclose(f);
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDONLY);
    uint64_t card_vt = base + 0x1a3bcf8;
    const size_t CH = 4 << 20;
    uint64_t *buf = malloc(CH + 128);
    int cnt = 0;
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH + 128 ? hi[r] - b : CH + 128;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            size_t lim = (len > CH ? CH : len) / 8;
            for (size_t i = 0; i + 12 < len / 8 && i < lim; i++) {
                if (buf[i] != VT) continue;
                int32_t *v = (int32_t *)&buf[i];
                int32_t ta = v[3], tb = v[4], player = v[6], x = v[10], y = v[11];
                if (x < 0 || x > 18000 || y < 0 || y > 32000 || ta < 0 || ta > 20000) continue;
                uint64_t cp = buf[i + 7], cv = 0; int32_t cid = -1;
                pread64(fd, &cv, 8, (off64_t)cp);
                if (cv == card_vt) pread64(fd, &cid, 4, (off64_t)(cp + 0x40));
                printf("%" PRIx64 " %d %d %d %d %d %d\n", b + i * 8, ta, tb, player, x, y, cid);
                cnt++;
            }
        }
    fprintf(stderr, "commands=%d\n", cnt);
    return 0;
}
