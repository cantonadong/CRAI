// CRAI 只读内存读取器（Null's Royale 16.635.1，arm64 libg.so，LDPlayer9）
// 偏移来源：configs/nulls_16.635.1.json（此处为编译期常量，改版本需同步）
//
// 流程：
//   1. 定位 libg 基址；扫描可写匿名内存找到双方国王塔（vtable 指纹），再找到同时指向两者的 roster（+0xe0/+0xe8）
//   2. 主循环按固定周期输出一行 JSON：双方圣水/手牌/队列/卡组/国王塔血量/出牌序号，以及实体列表
//   3. 实体：每 entity_rescan 帧在 libc_malloc 区域重扫一次 vtable 指纹，平时只复核已知地址
//   roster 失效（对局结束）时输出 {"battle":false} 并重新等待定位
//
// 用法: cr_reader <pid> [周期ms=33] [帧数=0(无限)] [实体重扫间隔帧=15]
#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define OFF_KING_VT 0x1a50840ull
#define OFF_UNIT_VT 0x1a4fbb8ull
#define OFF_CARD_VT 0x1a3bcf8ull
#define R_PLAYERS 0xe0
#define R_DECKS 0x88
#define P_SIDE 0x78
#define P_ELIXIR 0x2f8
#define P_HAND 0x210
#define P_QUEUE 0x220
#define P_QCOUNT 0x22c
#define P_SEQ 0x318
#define E_KIND 0x30
#define E_X 0x7c
#define E_Y 0x80
#define E_CARD 0xac
#define E_LVL 0x120

typedef struct { uint64_t lo, hi; int malloc_; } Reg;
static Reg regs[8192]; static int nregs;
static int fd;
static uint64_t base, king_vt, unit_vt, card_vt;

static int rd(uint64_t a, void *b, size_t n) { return pread64(fd, b, n, (off64_t)a) == (ssize_t)n; }
static uint64_t rp(uint64_t a) { uint64_t v = 0; if (!rd(a, &v, 8)) return 0; return v; }
static int okp(uint64_t p) { return p > 0x10000 && !(p & 7) && p < 0x800000000000ull; }
static double now_s(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

static void load_maps(const char *pid) {
    char path[64], line[512];
    snprintf(path, sizeof path, "/proc/%s/maps", pid);
    FILE *f = fopen(path, "r");
    nregs = 0;
    while (f && fgets(line, sizeof line, f) && nregs < 8192) {
        uint64_t a, b, off; char perms[8], name[256] = "";
        if (sscanf(line, "%" SCNx64 "-%" SCNx64 " %7s %" SCNx64 " %*s %*s %255[^\n]", &a, &b, perms, &off, name) < 4) continue;
        char *s = name; while (*s == ' ') s++;
        if (strstr(s, "libg.so") && off == 0 && !base) base = a;
        int anon = !*s || !strncmp(s, "[anon:", 6) || !strcmp(s, "[heap]");
        if (perms[0] == 'r' && perms[1] == 'w' && anon && !strstr(s, "stack") && b - a <= (512ull << 20))
            regs[nregs++] = (Reg){a, b, strstr(s, "libc_malloc") != NULL};
    }
    if (f) fclose(f);
}

// ---------- 实体 ----------
#define MAXE 1024
static uint64_t ents[MAXE]; static int nents;
static uint64_t *scanbuf;
#define CH (4u << 20)

static int valid_entity(uint64_t e, int32_t *f) {  // f: kind side x y card lvl
    uint8_t b[0x128];
    if (!rd(e, b, sizeof b)) return 0;
    uint64_t vt = *(uint64_t *)b;
    if (vt != unit_vt && vt != king_vt) return 0;
    f[0] = *(int32_t *)(b + E_KIND); f[1] = *(int32_t *)(b + P_SIDE);
    f[2] = *(int32_t *)(b + E_X); f[3] = *(int32_t *)(b + E_Y);
    f[4] = *(int32_t *)(b + E_CARD); f[5] = *(int32_t *)(b + E_LVL);
    return (f[1] == 0 || f[1] == 1) && f[2] >= 0 && f[2] <= 18000 && f[3] >= 0 && f[3] <= 32000;
}

static void hp_of(uint64_t e, int32_t *hp, int32_t *mx) {
    uint8_t k = 0; int32_t s = 0; *hp = *mx = -1;
    rd(e + 0x30, &k, 1); rd(e + 0x24, &s, 4);
    if (!(k & 4) || s < 3) return;
    uint64_t c = rp(rp(e + 0x18) + 0x10);
    if (okp(c)) { rd(c + 0x10, hp, 4); rd(c + 0x14, mx, 4); }
}

// 扫描指定区域列表，把实体地址写入 out，返回数量
static int scan_entities_into(const Reg *rs, int nr, int malloc_only, uint64_t *buf, uint64_t *out) {
    int n = 0;
    for (int r = 0; r < nr; r++) {
        if (malloc_only && !rs[r].malloc_) continue;
        for (uint64_t b = rs[r].lo; b < rs[r].hi; b += CH) {
            size_t len = rs[r].hi - b < CH ? rs[r].hi - b : CH;
            if (!rd(b, buf, len)) continue;
            for (size_t i = 0; i < len / 8; i++) {
                uint64_t v = buf[i];
                if (v != unit_vt && v != king_vt) continue;
                int32_t f[6];
                if (n < MAXE && valid_entity(b + i * 8, f)) out[n++] = b + i * 8;
            }
        }
    }
    return n;
}

static void rescan_entities(int malloc_only) {
    nents = scan_entities_into(regs, nregs, malloc_only, scanbuf, ents);
}

// ---------- 游戏内部实体列表 ----------
// list = *(*(roster+0x8)+0x40)；+0x8 数据指针，+0x10 容量(i32)，+0x14 数量(i32)
// 已验证：vtable 全扫描找到的实体 100% 在列表中；列表中偶有其他类型对象（疑似法术/投射物），这里只取部队/塔
#define R_ENTCTX 0x8
#define C_ENTLIST 0x40
static int read_entity_list(uint64_t roster, uint64_t *out) {
    uint64_t L = rp(rp(roster + R_ENTCTX) + C_ENTLIST);
    if (!okp(L)) return -1;
    uint64_t data = rp(L + 0x8);
    int32_t cap = 0, cnt = 0;
    rd(L + 0x10, &cap, 4); rd(L + 0x14, &cnt, 4);
    if (!okp(data) || cnt < 0 || cnt > cap || cnt > MAXE || cap > 100000) return -1;
    static uint64_t arr[MAXE];
    if (cnt && !rd(data, arr, 8 * (size_t)cnt)) return -1;
    int n = 0, towers = 0;
    for (int i = 0; i < cnt; i++) {
        uint64_t vt = rp(arr[i]);
        if (vt == unit_vt || vt == king_vt) { out[n++] = arr[i]; towers += vt == king_vt; }
    }
    return towers >= 1 ? n : -1;  // 正常对局至少有国王塔
}

// ---------- 后台扫描线程：持续重扫 malloc 区，主循环每帧只读已知地址 ----------
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static uint64_t shared_ents[MAXE]; static int shared_n; static long shared_gen;
static volatile int scan_enabled;

static void *scanner(void *arg) {
    (void)arg;
    static Reg local_regs[8192];
    static uint64_t local_out[MAXE];
    uint64_t *buf = malloc(CH);
    for (;;) {
        if (!scan_enabled) { usleep(50000); continue; }
        pthread_mutex_lock(&mu);
        int nr = nregs;
        memcpy(local_regs, regs, sizeof(Reg) * nr);
        pthread_mutex_unlock(&mu);
        int n = scan_entities_into(local_regs, nr, 1, buf, local_out);
        pthread_mutex_lock(&mu);
        memcpy(shared_ents, local_out, sizeof(uint64_t) * n);
        shared_n = n; shared_gen++;
        pthread_mutex_unlock(&mu);
        usleep(20000);
    }
    return NULL;
}

// ---------- roster ----------
// 上一局结束后旧对局对象仍留在内存且结构有效，所以可能有多个候选 roster。
// 收集全部候选，间隔 150ms 读两次双方原始圣水，选"圣水在变化"的（正在进行的对局）；
// 都没变化（例如双方圣水都满）时，选国王塔地址最新分配（地址最大）的一个作为兜底。
static uint64_t find_roster(void) {
    rescan_entities(0);
    uint64_t kings[16]; int nk = 0;
    for (int i = 0; i < nents && nk < 16; i++) if (rp(ents[i]) == king_vt) kings[nk++] = ents[i];
    if (nk < 2) return 0;
    uint64_t cand[8]; int nc = 0;
    for (int r = 0; r < nregs && nc < 8; r++)
        for (uint64_t b = regs[r].lo; b < regs[r].hi && nc < 8; b += CH) {
            size_t len = regs[r].hi - b < CH ? regs[r].hi - b : CH;
            if (!rd(b, scanbuf, len)) continue;
            for (size_t i = 0; i + 1 < len / 8 && nc < 8; i++)
                for (int x = 0; x < nk; x++) {
                    if (scanbuf[i] != kings[x]) continue;
                    for (int y = 0; y < nk; y++)
                        if (y != x && scanbuf[i + 1] == kings[y]) {
                            uint64_t ro = b + i * 8 - R_PLAYERS;
                            uint64_t d0 = rp(ro + R_DECKS);
                            int32_t cnt = 0;
                            if (okp(d0) && rd(d0 + 0x28, &cnt, 4) && cnt == 8 && nc < 8) cand[nc++] = ro;
                        }
                }
        }
    if (nc == 0) return 0;
    if (nc == 1) return cand[0];
    int32_t m1[8][2], m2[8][2];
    for (int c = 0; c < nc; c++)
        for (int s = 0; s < 2; s++) { m1[c][s] = -1; rd(rp(cand[c] + R_PLAYERS + 8 * s) + P_ELIXIR, &m1[c][s], 4); }
    usleep(150000);
    uint64_t best = 0, newest = 0, newest_ro = 0;
    for (int c = 0; c < nc; c++) {
        int changed = 0;
        for (int s = 0; s < 2; s++) {
            m2[c][s] = -1; rd(rp(cand[c] + R_PLAYERS + 8 * s) + P_ELIXIR, &m2[c][s], 4);
            changed |= m2[c][s] != m1[c][s];
        }
        uint64_t k = rp(cand[c] + R_PLAYERS);
        if (k > newest) { newest = k; newest_ro = cand[c]; }
        if (changed && !best) best = cand[c];
    }
    fprintf(stderr, "roster candidates=%d, live=0x%" PRIx64 "\n", nc, best);
    return best ? best : newest_ro;
}

static int roster_ok(uint64_t ro) {
    uint64_t k0 = rp(ro + R_PLAYERS), k1 = rp(ro + R_PLAYERS + 8);
    return okp(k0) && okp(k1) && rp(k0) == king_vt && rp(k1) == king_vt;
}

// 旧对局检测：双方原始圣水 3 秒未变化且不是都满（100000）时，认为当前 roster 已是遗留对象
static int32_t st_m[2] = {-2, -2};
static double st_t;
static uint64_t st_ro;
static int stale_roster(uint64_t ro, double now) {
    int32_t m[2] = {-1, -1};
    for (int s = 0; s < 2; s++) rd(rp(ro + R_PLAYERS + 8 * s) + P_ELIXIR, &m[s], 4);
    if (ro != st_ro || m[0] != st_m[0] || m[1] != st_m[1]) {
        st_ro = ro; st_m[0] = m[0]; st_m[1] = m[1]; st_t = now;
        return 0;
    }
    if (m[0] >= 100000 && m[1] >= 100000) { st_t = now; return 0; }
    return now - st_t > 3.0;
}

static int32_t card_of_elem(uint64_t el) {
    int32_t id = -1;
    if (!okp(el)) return -1;
    if (rp(el) == card_vt) { rd(el + 0x40, &id, 4); return id; }
    uint64_t c = rp(el + 0x10);
    if (okp(c) && rp(c) == card_vt) rd(c + 0x40, &id, 4);
    return id;
}

static void emit_player(uint64_t ro, int i) {
    uint64_t k = rp(ro + R_PLAYERS + 8 * i), deck = rp(ro + R_DECKS + 8 * i);
    int32_t side = -1, mana = 0, h[4] = {-1, -1, -1, -1}, q[4] = {-1, -1, -1, -1}, qc = 0, hp, mx, ids[8];
    uint64_t seq = 0;
    rd(k + P_SIDE, &side, 4); rd(k + P_ELIXIR, &mana, 4); rd(k + P_SEQ, &seq, 8);
    rd(rp(k + P_HAND), h, 16); rd(k + P_QCOUNT, &qc, 4);
    if (qc > 0) rd(rp(k + P_QUEUE), q, 4 * (qc > 4 ? 4 : qc));
    hp_of(k, &hp, &mx);
    uint64_t data = rp(deck + 0x20);
    for (int d = 0; d < 8; d++) ids[d] = card_of_elem(rp(data + 8 * d));
    int32_t last_card = -1;  // 最近打出的卡牌：*(player+0x258) 为卡牌对象，ID 在 +0x40
    uint64_t lc = rp(k + 0x258);
    if (okp(lc) && rp(lc) == card_vt) rd(lc + 0x40, &last_card, 4);
    printf("{\"side\":%d,\"elixir\":%.4f,\"seq\":%" PRIu64 ",\"last_card\":%d,\"king_hp\":[%d,%d],\"hand\":[%d,%d,%d,%d],\"next\":[%d,%d,%d,%d],"
           "\"deck\":[%d,%d,%d,%d,%d,%d,%d,%d]}",
           side, mana / 10000.0, seq, last_card, hp, mx, h[0], h[1], h[2], h[3], q[0], q[1], q[2], q[3],
           ids[0], ids[1], ids[2], ids[3], ids[4], ids[5], ids[6], ids[7]);
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s pid [period_ms] [frames] [rescan]\n", argv[0]); return 1; }
    int period = argc > 2 ? atoi(argv[2]) : 33;
    long frames = argc > 3 ? atol(argv[3]) : 0;
    int rescan = argc > 4 ? atoi(argv[4]) : 15;
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    fd = open(path, O_RDONLY);
    load_maps(argv[1]);
    if (fd < 0 || !base) { fprintf(stderr, "cannot open process\n"); return 1; }
    king_vt = base + OFF_KING_VT; unit_vt = base + OFF_UNIT_VT; card_vt = base + OFF_CARD_VT;
    scanbuf = malloc(CH);
    setvbuf(stdout, NULL, _IOFBF, 1 << 16);

    uint64_t roster = 0;
    double t0 = now_s();
    long seen_gen = 0;
    static uint64_t list_buf[MAXE];
    int list_n = -1;
    if (rescan > 0) { pthread_t th; pthread_create(&th, NULL, scanner, NULL); }
    for (long fr = 0; frames == 0 || fr < frames; fr++) {
        double ts = now_s();
        if (!roster || !roster_ok(roster)) {
            scan_enabled = 0;
            pthread_mutex_lock(&mu);
            load_maps(argv[1]);
            pthread_mutex_unlock(&mu);
            double a = now_s();
            roster = find_roster();
            if (!roster) {
                printf("{\"t\":%.3f,\"battle\":false}\n", ts - t0); fflush(stdout);
                usleep(500000);
                continue;
            }
            fprintf(stderr, "roster=0x%" PRIx64 " located in %.2fs, %d entities\n", roster, now_s() - a, nents);
            scan_enabled = rescan > 0;
        } else if (stale_roster(roster, ts)) {
            fprintf(stderr, "roster 0x%" PRIx64 " looks stale (elixir frozen), relocating\n", roster);
            roster = 0;
            continue;
        } else if ((list_n = read_entity_list(roster, list_buf)) >= 0) {
            // 优先：直接读游戏内部实体列表（实时、完整）
            memcpy(ents, list_buf, sizeof(uint64_t) * list_n);
            nents = list_n;
            scan_enabled = 0;
        } else if (rescan > 0) {
            // 兜底：后台线程有新一轮扫描结果时，替换已知实体表
            scan_enabled = 1;
            pthread_mutex_lock(&mu);
            if (shared_gen != seen_gen) {
                memcpy(ents, shared_ents, sizeof(uint64_t) * shared_n);
                nents = shared_n; seen_gen = shared_gen;
            }
            pthread_mutex_unlock(&mu);
        }
        printf("{\"t\":%.3f,\"battle\":true,\"roster\":\"%" PRIx64 "\",\"players\":[", ts - t0, roster);
        emit_player(roster, 0); printf(","); emit_player(roster, 1);
        printf("],\"entities\":[");
        int first = 1;
        for (int i = 0; i < nents; i++) {
            int32_t f[6], hp, mx;
            if (!valid_entity(ents[i], f)) { ents[i] = ents[--nents]; i--; continue; }
            hp_of(ents[i], &hp, &mx);
            printf("%s{\"id\":\"%" PRIx64 "\",\"kind\":%d,\"side\":%d,\"x\":%d,\"y\":%d,\"card\":%d,\"lvl\":%d,\"hp\":%d,\"max_hp\":%d}",
                   first ? "" : ",", ents[i], f[0], f[1], f[2], f[3], f[4], f[5] + 1, hp, mx);
            first = 0;
        }
        printf("],\"ent_src\":\"%s\",\"read_ms\":%.2f}\n", list_n >= 0 ? "list" : "scan", (now_s() - ts) * 1000);
        fflush(stdout);
        double el = (now_s() - ts) * 1000;
        if (el < period) usleep((useconds_t)((period - el) * 1000));
    }
    return 0;
}
