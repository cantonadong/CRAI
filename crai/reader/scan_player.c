// 只读扫描：在游戏进程的可写匿名内存中寻找 runtime player 对象候选。
// 判据（16.635.1 静态分析结论）：
//   player+0x210 -> int 数组指针（手牌 deck index），player+0x21c == 4（数量）
//   数组内 4 个值互不相同且在 [0,7]
//   player+0x2f8 原始圣水在 [0,100000]
// 同时打印推测的下一张队列：player+0x220 指针 / player+0x22c 数量。
// 用法: scan_player <pid>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct { uint64_t lo, hi; } Region;

static Region regs[65536];
static int nregs;
static int memfd;

static int readable(uint64_t p, uint64_t len) {
    for (int i = 0; i < nregs; i++)
        if (p >= regs[i].lo && p + len <= regs[i].hi) return 1;
    return 0;
}

static int rd(uint64_t addr, void *buf, size_t n) {
    return pread64(memfd, buf, n, (off64_t)addr) == (ssize_t)n;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s pid\n", argv[0]); return 1; }
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/maps", argv[1]);
    FILE *f = fopen(path, "r");
    if (!f) { perror("maps"); return 1; }
    char line[512];
    Region scan[65536]; int nscan = 0;
    while (fgets(line, sizeof line, f)) {
        uint64_t lo, hi; char perms[8]; char name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %*s %*s %*s %255[^\n]", &lo, &hi, perms, name) < 3) continue;
        if (perms[0] != 'r') continue;
        if (nregs < 65536) regs[nregs++] = (Region){lo, hi};
        char *n = name; while (*n == ' ') n++;
        int anon = (*n == 0) || strncmp(n, "[anon:", 6) == 0 || strcmp(n, "[heap]") == 0;
        if (perms[1] == 'w' && anon && strstr(n, "stack") == NULL && hi - lo <= (512ull << 20) && nscan < 65536)
            scan[nscan++] = (Region){lo, hi};
    }
    fclose(f);

    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    memfd = open(path, O_RDONLY);
    if (memfd < 0) { perror("mem"); return 1; }

    const size_t CH = 4 << 20;
    uint8_t *buf = malloc(CH + 0x400);
    uint64_t total = 0; int found = 0;
    for (int r = 0; r < nscan; r++) {
        for (uint64_t base = scan[r].lo; base < scan[r].hi; base += CH) {
            uint64_t end = base + CH + 0x300; if (end > scan[r].hi) end = scan[r].hi;
            size_t n = end - base;
            if (n < 0x300 || !rd(base, buf, n)) continue;
            total += n;
            size_t lim = (base + CH < scan[r].hi ? CH : n - 0x300);
            for (size_t off = 0; off + 0x300 <= n && off < lim; off += 8) {
                int32_t cnt = *(int32_t *)(buf + off + 0x21c);
                if (cnt != 4) continue;
                int32_t mana = *(int32_t *)(buf + off + 0x2f8);
                if (mana < 0 || mana > 100000) continue;
                uint64_t hp = *(uint64_t *)(buf + off + 0x210);
                if (hp < 0x10000 || (hp & 3) || !readable(hp, 16)) continue;
                int32_t h[4];
                if (!rd(hp, h, 16)) continue;
                int ok = 1, seen = 0;
                for (int i = 0; i < 4; i++) {
                    if (h[i] < 0 || h[i] > 7 || (seen >> h[i]) & 1) { ok = 0; break; }
                    seen |= 1 << h[i];
                }
                if (!ok) continue;
                uint64_t qp = *(uint64_t *)(buf + off + 0x220);
                int32_t qc = *(int32_t *)(buf + off + 0x22c);
                int32_t q[4] = {-9, -9, -9, -9};
                if (qc > 0 && qc <= 8 && readable(qp, 16)) rd(qp, q, 16);
                printf("player=0x%" PRIx64 " mana=%d(%.2f) hand=[%d,%d,%d,%d] queue_cnt=%d queue=[%d,%d,%d,%d]\n",
                       base + off, mana, mana / 10000.0, h[0], h[1], h[2], h[3], qc, q[0], q[1], q[2], q[3]);
                if (++found > 200) goto done;
            }
        }
    }
done:
    fprintf(stderr, "scanned %d regions, %.1f MiB, %d candidates\n", nscan, total / 1048576.0, found);
    return 0;
}
