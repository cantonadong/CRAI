// 只读：打印对局双方的卡组容器（roster+0x88/+0x90）与手牌/队列。
// 每个卡组槽位：元素指针 E、E 的前 0x40 字节（i32）、卡牌数据指针（E+0x10）与卡牌 ID（+0x40）
// 用法: dump_decks <pid> <roster hex>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int fd;
static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; rd(a, &v, 8); return v; }

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    uint64_t ro = strtoull(argv[2], NULL, 16);
    for (int s = 0; s < 2; s++) {
        uint64_t k = rp(ro + 0xe0 + 8 * s), deck = rp(ro + 0x88 + 8 * s);
        int32_t side = -1, h[4] = {0}, q[4] = {0}, qc = 0, cnt = 0;
        uint64_t seq = 0;
        rd(k + 0x78, &side, 4); rd(rp(k + 0x210), h, 16); rd(k + 0x22c, &qc, 4); rd(rp(k + 0x220), q, 16); rd(k + 0x318, &seq, 8);
        rd(deck + 0x28, &cnt, 4);
        printf("player[%d] king=0x%" PRIx64 " side=%d seq=%" PRIu64 " hand=[%d,%d,%d,%d] queue(%d)=[%d,%d,%d,%d] deck=0x%" PRIx64 " n=%d"
               " hand_ptr=0x%" PRIx64 " queue_ptr=0x%" PRIx64 "\n",
               s, k, side, seq, h[0], h[1], h[2], h[3], qc, q[0], q[1], q[2], q[3], deck, cnt, rp(k + 0x210), rp(k + 0x220));
        uint64_t data = rp(deck + 0x20);
        for (int d = 0; d < cnt && d < 8; d++) {
            uint64_t e = rp(data + 8 * d), c = rp(e + 0x10);
            int32_t cid = -1, raw[16];
            rd(c + 0x40, &cid, 4); rd(e, raw, sizeof raw);
            printf("  deck[%d] elem=0x%" PRIx64 " card=0x%" PRIx64 " id=%d raw:", d, e, c, cid);
            for (int i = 0; i < 16; i++) printf(" %d", raw[i]);
            printf("\n");
        }
    }
    return 0;
}
