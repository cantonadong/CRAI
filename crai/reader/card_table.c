// 只读：扫描全局卡牌数据对象（vtable = libg+0x1a3bcf8），输出 "卡牌ID 指针"（每个 ID 取第一个）。
// 用法: card_table <pid>
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
    uint64_t vt = base + 0x1a3bcf8;
    const size_t CH = 4 << 20;
    uint64_t *buf = malloc(CH + 0x48);
    static int seen_ids[4096]; int ns = 0;
    for (int r = 0; r < n; r++)
        for (uint64_t b = lo[r]; b < hi[r]; b += CH) {
            size_t len = hi[r] - b < CH + 0x48 ? hi[r] - b : CH + 0x48;
            if (pread64(fd, buf, len, (off64_t)b) != (ssize_t)len) continue;
            size_t lim = (len > CH ? CH : len) / 8;
            for (size_t i = 0; i < lim && i + 9 < len / 8; i++) {
                if (buf[i] != vt) continue;
                int32_t id = ((int32_t *)&buf[i])[16];
                if (!((id >= 26000000 && id < 29000000) || (id >= 159000000 && id < 160000000))) continue;
                int dup = 0;
                for (int k = 0; k < ns; k++) if (seen_ids[k] == id) { dup = 1; break; }
                if (dup || ns >= 4096) continue;
                seen_ids[ns++] = id;
                printf("%d %" PRIx64 "\n", id, b + i * 8);
            }
        }
    fprintf(stderr, "cards=%d\n", ns);
    return 0;
}
