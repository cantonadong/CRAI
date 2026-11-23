// 最小 probe：冒充 libsentry-android.so（原库改名 libsentry-android-real.so，由本库 DT_NEEDED 加载）。
// 后台线程等 libg 映射出现后，按 hooks[] 表在函数开头写 16 字节跳转。
//  - GameStateStep：计数，验证钩子可用（每 200 次写一次 logcat，tag CRProbe）
//  - ReplayJsonSetter：把游戏收到的回放 JSON 原样存到 /data/data/<pkg>/files/crprobe_replay_<n>.json
// 地址见 configs/fl_engine_16.635.1.json。编译部署：scripts/probe_min.ps1
#define _GNU_SOURCE 1
#include <android/log.h>
#include <link.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define TAG "CRProbe"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define DUMP_DIR "/data/data/nullsroyale.rel.free/files"

static uintptr_t g_libg;

// ---------- 钩子 ----------
static void (*g_orig_step)(void *);
static volatile long g_step_calls;

static void step_hook(void *self) {
  long n = __atomic_add_fetch(&g_step_calls, 1, __ATOMIC_RELAXED);
  if (n == 1 || n % 2000 == 0) LOGI("GameStateStep hit #%ld self=%p", n, self);
  g_orig_step(self);
}

// libg 字符串：+0 缓存长度，+4 字节长度，+8 内联内容（<8 字节）或指针
struct LibgString {
  int32_t cached_len;
  int32_t byte_len;
  union {
    char inline_bytes[8];
    const char *ptr;
  } s;
};

static void (*g_orig_replay_setter)(void *, void *);
static volatile int g_dump_index;

static void replay_setter_hook(void *self, void *str) {
  const struct LibgString *ls = (const struct LibgString *)str;
  if (ls && ls->byte_len > 0 && ls->byte_len < (64 << 20)) {
    const char *text = ls->byte_len < 8 ? ls->s.inline_bytes : ls->s.ptr;
    int n = __atomic_add_fetch(&g_dump_index, 1, __ATOMIC_RELAXED);
    char path[256];
    snprintf(path, sizeof path, DUMP_DIR "/crprobe_replay_%d.json", n);
    FILE *f = fopen(path, "wb");
    if (f) {
      fwrite(text, 1, (size_t)ls->byte_len, f);
      fclose(f);
    }
    LOGI("replay setter self=%p bytes=%d -> %s (%s)", self, ls->byte_len, path, f ? "ok" : "open failed");
  } else {
    LOGI("replay setter self=%p str=%p (unexpected layout)", self, str);
  }
  g_orig_replay_setter(self, str);
}

// ---------- 离线引擎（headless 重演） ----------
// 触发：出现 DUMP_DIR/crprobe_in.json（游戏原生回放 JSON：battle/cmd/evt/endTick/rndSeed）。
// 流程同 16.635.1 的 NativeReplayRunner(0xd9ae04)：new(0x240) → ctor → setup → 注册监听器 → load；然后循环 step。
// 每个新 tick 往 DUMP_DIR/crprobe_out.jsonl 写一行（格式同 cr_reader：players + entities），结束写 crprobe_done。
#define OFF_CTX_A 0x878c00
#define OFF_CTX_B 0xff253c
#define OFF_NEW 0x198db50          /* PLT operator new */
#define OFF_GSM_CTOR 0x11d1e68
#define OFF_GSM_SETUP 0x11d225c
#define OFF_LISTENER_INIT 0x10cab40
#define OFF_LISTENER_VT_GOT 0x1a9a0d0
#define OFF_LISTENER_ADD 0x108d23c
#define OFF_JSON_PARSE 0x139f838
#define OFF_GSM_LOAD 0x11d2dc4
#define OFF_GSM_TICK 0x11d32f0
#define OFF_DATA_BY_ID 0xefe42c
#define OFF_TILEMAP_LOOKUP 0xf2edec
#define OFF_TILEMAP_ENSURE 0x8ae750
#define OFF_KING_VT 0x1a50840
#define OFF_UNIT_VT 0x1a4fbb8
#define OFF_CARD_VT 0x1a3bcf8
#define GSM_SIZE 0x240
#define OFF_GSM_DESTROY 0x11d2388
#define OFF_GSM_BASE_DESTROY 0x9fe0b0
#define OFF_DELETE 0x198db60       /* PLT operator delete */

static long rss_kb(void) {
  FILE *f = fopen("/proc/self/status", "r");
  char line[128];
  long v = -1;
  while (f && fgets(line, sizeof line, f))
    if (!strncmp(line, "VmRSS:", 6)) { v = strtol(line + 6, NULL, 10); break; }
  if (f) fclose(f);
  return v;
}

// JSON 对象的析构在虚表第 1 项（同 ReplayJsonSetter 释放旧对象的方式）
static void release_json(void *obj) {
  if (!obj) return;
  void **vt = *(void ***)obj;
  if (vt && vt[1]) ((void (*)(void *))vt[1])(obj);
}

#define FN(type, off) ((type)(g_libg + (off)))
typedef void *(*fn_p_v)(void);
typedef void *(*fn_p_sz)(size_t);
typedef void (*fn_ctor)(void *, int32_t, void *, void *, int32_t);
typedef void (*fn_v_pi)(void *, int32_t);
typedef void (*fn_v_p)(void *);
typedef void (*fn_v_pp)(void *, void *);
typedef void *(*fn_json)(void *, void *, int32_t);
typedef int (*fn_load)(void *, void *, void *, void *);
typedef int32_t (*fn_i_p)(void *);
typedef void *(*fn_p_pi)(void *, int32_t);
typedef void *(*fn_p_p)(void *);

static int okp(uintptr_t p) { return p > 0x10000 && !(p & 7) && p < 0x800000000000ull; }

// 安全读：通过 process_vm_readv 读自己进程，地址无效时返回 0 而不是触发段错误
#include <sys/uio.h>
static int safe_read(uintptr_t addr, void *buf, size_t n) {
  struct iovec local = {buf, n}, remote = {(void *)addr, n};
  return process_vm_readv(getpid(), &local, 1, &remote, 1, 0) == (ssize_t)n;
}
#define RP(a) (okp((uintptr_t)(a)) ? *(uintptr_t *)(uintptr_t)(a) : 0)
#define RI(a) (*(int32_t *)(uintptr_t)(a))

static void hp_of(uintptr_t e, int32_t *hp, int32_t *mx) {
  *hp = *mx = -1;
  uint8_t k = *(uint8_t *)(e + 0x30);
  int32_t s = RI(e + 0x24);
  if (!(k & 4) || s < 3) return;
  uintptr_t c = RP(RP(e + 0x18) + 0x10);
  if (okp(c)) { *hp = RI(c + 0x10); *mx = RI(c + 0x14); }
}

#define OFF_CARD_SELECTION_BUILD 0xf671a0
extern void cr_call_card_selection_build(void *battle_deck_slot, void *player, int alternate, void *result, void *target);

static void emit_player(FILE *o, uintptr_t world, int i) {
  uintptr_t k = RP(world + 0xe0 + 8 * i);
  if (!okp(k)) { fprintf(o, "null"); return; }
  int32_t h[4] = {-1, -1, -1, -1}, q[4] = {-1, -1, -1, -1}, hp, mx;
  uintptr_t hp_arr = RP(k + 0x210), q_arr = RP(k + 0x220);
  int32_t qc = RI(k + 0x22c);
  if (okp(hp_arr)) memcpy(h, (void *)hp_arr, 16);
  if (okp(q_arr) && qc > 0) memcpy(q, (void *)q_arr, 4 * (qc > 4 ? 4 : qc));
  hp_of(k, &hp, &mx);
  // 卡组：world+0x88+8i → 容器 +0x20 数据 → 元素 → +0x10 卡牌对象 → +0x40 ID
  int32_t ids[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  uintptr_t deck = RP(world + 0x88 + 8 * i), data = okp(deck) ? RP(deck + 0x20) : 0;
  uintptr_t card_vt = g_libg + OFF_CARD_VT;
  for (int d = 0; d < 8 && okp(data); d++) {
    uintptr_t el = RP(data + 8 * d);
    if (!okp(el)) continue;
    uintptr_t c = RP(el) == card_vt ? el : RP(el + 0x10);
    if (okp(c) && RP(c) == card_vt) ids[d] = RI(c + 0x40);
  }
  // 手牌的 pd（cardParameter）：用游戏自己的 CardSelection::build(卡组元素, player, 0) 现算，
  // 进化是否就绪、英雄形态都编码在里面（低 4 位：0 普通 1 进化 2 英雄）
  uint32_t pd[4] = {0, 0, 0, 0};
  for (int s = 0; s < 4 && okp(data); s++) {
    if (h[s] < 0 || h[s] >= 8) continue;
    uintptr_t slot = RP(data + 8 * h[s]);
    if (!okp(slot)) continue;
    struct { void *card_data; void *secondary; uint32_t packed; uint32_t pad; } sel = {0};
    cr_call_card_selection_build((void *)slot, (void *)k, 0, &sel, (void *)(g_libg + OFF_CARD_SELECTION_BUILD));
    pd[s] = sel.packed;
  }
  // 英雄/冠军技能控制器：player+0x3a0 / +0x3a8（16 版与 15 版相同，2026-11-22 实测），要求控制器 +0x20 回指 player。
  //   +0x18 动作数据（+0x40 数据全局 ID），+0x78 剩余冷却 ms，+0x7c 总冷却，+0x80 剩余次数（-1 = 无），
  //   +0x90 角色数据（+0x40 数据全局 ID，+0x710 → 技能数据，名字长度 +0x2c、内容 +0x30），+0x98 按钮状态，
  //   +0xa0 → 成员向量（+0x00 数组，+0x0c 数量），成员对象 +0x08 = 对象 ID。
  char abil[512];
  int ab = snprintf(abil, sizeof abil, ",\"abil\":[");
  for (int s = 0, firsta = 1; s < 2; s++) {
    uintptr_t c = 0, back = 0, ch = 0, act = 0, vec = 0, asset = 0;
    if (!safe_read(k + 0x3a0 + 8 * s, &c, 8) || !okp(c) || !safe_read(c + 0x20, &back, 8) || back != k) continue;
    int32_t st = -1, cd = -1, tot = -1, chg = -2, adg = -1, cdg = -1, cnt = 0, len = 0;
    safe_read(c + 0x90, &ch, 8);
    if (!okp(ch)) continue;
    safe_read(c + 0x98, &st, 4); safe_read(c + 0x78, &cd, 4); safe_read(c + 0x7c, &tot, 4); safe_read(c + 0x80, &chg, 4);
    if (safe_read(c + 0x18, &act, 8) && okp(act)) safe_read(act + 0x40, &adg, 4);
    safe_read(ch + 0x40, &cdg, 4);
    char nm[129] = "";
    if (safe_read(ch + 0x710, &asset, 8) && okp(asset) && safe_read(asset + 0x2c, &len, 4) && len > 0 && len <= 128) {
      uintptr_t src = asset + 0x30;
      if (len >= 8 && !safe_read(asset + 0x30, &src, 8)) src = 0;
      if (!src || !safe_read(src, nm, (size_t)len)) nm[0] = 0; else nm[len] = 0;
      for (int q = 0; nm[q]; q++)
        if (!((nm[q] >= 'a' && nm[q] <= 'z') || (nm[q] >= 'A' && nm[q] <= 'Z') || (nm[q] >= '0' && nm[q] <= '9') || nm[q] == '_')) { nm[0] = 0; break; }
    }
    int32_t mem[4] = {0, 0, 0, 0};
    uintptr_t items = 0;
    if (safe_read(c + 0xa0, &vec, 8) && okp(vec) && safe_read(vec + 0x0c, &cnt, 4) && cnt > 0 && cnt <= 4 &&
        safe_read(vec, &items, 8) && okp(items)) {
      for (int q = 0; q < cnt; q++) {
        uintptr_t m = 0;
        if (safe_read(items + 8 * q, &m, 8) && okp(m)) safe_read(m + 8, &mem[q], 4);
      }
    } else cnt = 0;
    ab += snprintf(abil + ab, sizeof abil - ab, "%s{\"s\":%d,\"st\":%d,\"cd\":%d,\"tot\":%d,\"chg\":%d,\"adg\":%d,\"cdg\":%d,"
                   "\"n\":\"%s\",\"m\":[%d,%d,%d,%d],\"mc\":%d}",
                   firsta ? "" : ",", s + 1, st, cd, tot, chg, adg, cdg, nm, mem[0], mem[1], mem[2], mem[3], cnt);
    firsta = 0;
  }
  snprintf(abil + ab, sizeof abil - ab, "]");
  fprintf(o, "{\"side\":%d,\"acct\":%d,\"elixir\":%.4f,\"seq\":%lld,\"king_hp\":[%d,%d],\"hand\":[%d,%d,%d,%d],"
          "\"pd\":[%u,%u,%u,%u],\"next\":[%d,%d,%d,%d],\"deck\":[%d,%d,%d,%d,%d,%d,%d,%d]%s}",
          RI(k + 0x78), okp(RP(world + 0x30 + 8 * i)) ? RI(RP(world + 0x30 + 8 * i) + 4) : 0, RI(k + 0x2f8) / 10000.0,
          (long long)*(int64_t *)(k + 0x318), hp, mx, h[0], h[1], h[2], h[3], pd[0], pd[1], pd[2], pd[3],
          q[0], q[1], q[2], q[3], ids[0], ids[1], ids[2], ids[3], ids[4], ids[5], ids[6], ids[7], abil);
}

// 实体列表：优先 FirstLight 的 king+0x10 → object manager；否则用我们读取器的 *(*(world+0x8)+0x40)
static int entity_list(uintptr_t world, uintptr_t **data, int *src) {
  uintptr_t cands[2] = {RP(RP(world + 0xe0) + 0x10), RP(RP(world + 0x8) + 0x40)};
  for (int c = 0; c < 2; c++) {
    uintptr_t L = cands[c];
    if (!okp(L)) continue;
    uintptr_t d = RP(L + 0x8);
    int32_t cap = RI(L + 0x10), cnt = RI(L + 0x14);
    if (cnt < 0 || cnt > cap || cap > 100000 || (cnt && !okp(d))) continue;
    *data = (uintptr_t *)d;
    *src = c;
    return cnt;
  }
  return -1;
}

static void emit_frame(FILE *o, int32_t tick, uintptr_t world) {
  uintptr_t king_vt = g_libg + OFF_KING_VT, unit_vt = g_libg + OFF_UNIT_VT;
  fprintf(o, "{\"tick\":%d,\"players\":[", tick);
  emit_player(o, world, 0);
  fputc(',', o);
  emit_player(o, world, 1);
  fprintf(o, "],\"entities\":[");
  uintptr_t *arr = NULL;
  int src = -1, n = entity_list(world, &arr, &src), first = 1, other = 0;
  for (int i = 0; i < n; i++) {
    uintptr_t e = arr[i];
    if (!okp(e)) continue;
    uintptr_t vt = RP(e);
    if (vt != unit_vt && vt != king_vt) { other++; continue; }
    int32_t hp, mx;
    hp_of(e, &hp, &mx);
    // 数据对象（LogicData）在 +0x48，数据全局 ID 在其 +0x40（16 版；15 版 +0x10），即 FirstLight 的 dataGlobalId。
    // 例：野猪 34000021、加农炮 35000002、英雄火枪手 130283371、进化加农炮 140000000（2026-11-21 实测）
    uintptr_t dat = RP(e + 0x48);
    int32_t dgid = -1;
    if (okp(dat)) safe_read(dat + 0x40, &dgid, 4);
    // 攻击组件：组件数组 +0x18 的第 0 个（第 2 个是血量，hp_of 已在用），要求 comp+0x08 == 实体本身。
    // 字段（15 版布局，RoyaleHarness runtime_snapshot.inc / FirstLight phase_runtime_layout.h）：
    //   +0x10 目标对象指针，+0x20 攻击阶段，+0x24 攻击时间轴 ms，+0x28 装填剩余 ms。
    // 输出 "atk":[目标 gid（0=无目标，-1=目标不在列表里）, 阶段, 时间轴, 装填]，组件不合格时不输出。
    char atk[96] = "";
    {
      uintptr_t parts = 0, comp = 0, owner = 0, tgt = 0;
      int32_t cnt = 0, cap = 0, stage = -2, tl = -1, ld = -1;
      if (safe_read(e + 0x18, &parts, 8) && safe_read(e + 0x20, &cap, 4) && safe_read(e + 0x24, &cnt, 4) &&
          okp(parts) && cnt > 0 && cnt <= cap && cap <= 32 && safe_read(parts, &comp, 8) && okp(comp) &&
          safe_read(comp + 8, &owner, 8) && owner == e && safe_read(comp + 0x10, &tgt, 8) &&
          safe_read(comp + 0x20, &stage, 4) && safe_read(comp + 0x24, &tl, 4) && safe_read(comp + 0x28, &ld, 4)) {
        int32_t tg = 0;
        if (tgt) {
          tg = -1;
          for (int j = 0; j < n; j++)
            if (arr[j] == tgt) { tg = RI(tgt + 0x08); break; }
        }
        snprintf(atk, sizeof atk, ",\"atk\":[%d,%d,%d,%d]", tg, stage, tl, ld);
      }
    }
    fprintf(o, "%s{\"id\":\"%lx\",\"gid\":%d,\"dgid\":%d,\"kind\":%d,\"side\":%d,\"x\":%d,\"y\":%d,\"card\":%d,\"lvl\":%d,"
            "\"hp\":%d,\"max_hp\":%d%s}",
            first ? "" : ",", (unsigned long)e, RI(e + 0x08), dgid, RI(e + 0x30), RI(e + 0x78),
            RI(e + 0x7c), RI(e + 0x80), RI(e + 0xac), RI(e + 0x120) + 1, hp, mx, atk);
    first = 0;
  }
  // 其他对象（投射物、区域效果等）：只给基本字段，供 FirstLight 适配器按 dgid 过滤使用
  fprintf(o, "],\"others\":[");
  first = 1;
  for (int i = 0; i < n; i++) {
    uintptr_t e = arr[i];
    if (!okp(e)) continue;
    uintptr_t vt = RP(e);
    if (vt == unit_vt || vt == king_vt) continue;
    uintptr_t dat = RP(e + 0x48);
    int32_t dgid = -1;
    if (okp(dat)) safe_read(dat + 0x40, &dgid, 4);
    fprintf(o, "%s{\"gid\":%d,\"dgid\":%d,\"vt\":%lu,\"side\":%d,\"x\":%d,\"y\":%d,\"card\":%d}", first ? "" : ",",
            RI(e + 0x08), dgid, (unsigned long)(vt - g_libg), RI(e + 0x78), RI(e + 0x7c), RI(e + 0x80), RI(e + 0xac));
    first = 0;
  }
  fprintf(o, "],\"list_src\":%d,\"list_n\":%d,\"other\":%d}\n", src, n, other);
}

static char *read_file(const char *path, long *len) {
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;
  fseek(f, 0, SEEK_END);
  *len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *b = malloc((size_t)*len + 1);
  if (b && fread(b, 1, (size_t)*len, f) != (size_t)*len) { free(b); b = NULL; }
  fclose(f);
  if (b) b[*len] = 0;
  return b;
}

// 一局离线对局：管理器 + 解析后的开局 JSON。引擎同一时间只跑一局（g_engine_mu 保护）。
struct Engine {
  void *m;
  void *root;
  char *json;   // 原文（JSON 对象可能引用，释放 root 后才能 free）
};
static pthread_mutex_t g_engine_mu = PTHREAD_MUTEX_INITIALIZER;

static void *parse_json(const char *text, long len) {
  struct LibgString s = {-1, (int32_t)len, {{0}}};
  if (len < 8) memcpy(s.s.inline_bytes, text, (size_t)len);
  else s.s.ptr = text;
  return FN(fn_json, OFF_JSON_PARSE)(&s, NULL, 100);
}

static void engine_destroy(struct Engine *g) {
  // 顺序同 FirstLight：destroy → base destroy → delete → 释放 JSON。不清理的话几百局后内存耗尽、系统重启。
  if (g->m) {
    FN(fn_v_p, OFF_GSM_DESTROY)(g->m);
    FN(fn_v_p, OFF_GSM_BASE_DESTROY)(g->m);
    FN(fn_v_p, OFF_DELETE)(g->m);
  }
  release_json(g->root);
  free(g->json);
  memset(g, 0, sizeof *g);
}

// 按开局 JSON 新建一局并加载（json 的所有权交给 Engine）。成功返回 1。
static int engine_create(struct Engine *g, char *json, long len) {
  memset(g, 0, sizeof *g);
  g->json = json;
  // 地点 tilemap 预热
  int32_t location = 0;
  const char *lp = strstr(json, "\"location\":");
  if (lp) location = (int32_t)strtol(lp + 11, NULL, 10);
  void *ctx_a = FN(fn_p_v, OFF_CTX_A)();
  void *ctx_b = FN(fn_p_v, OFF_CTX_B)();
  void *loc = location ? FN(fn_p_pi, OFF_DATA_BY_ID)(ctx_a, location) : NULL;
  void *tm = loc ? FN(fn_p_p, OFF_TILEMAP_LOOKUP)(loc) : NULL;
  if (loc && !tm) {
    FN(fn_v_p, OFF_TILEMAP_ENSURE)(loc);
    tm = FN(fn_p_p, OFF_TILEMAP_LOOKUP)(loc);
  }
  if (!tm) {
    LOGI("engine: tilemap not ready (location=%d)", location);
    return 0;
  }
  g->root = parse_json(json, len);
  if (!g->root) {
    LOGI("engine: json parse failed");
    return 0;
  }
  void *m = FN(fn_p_sz, OFF_NEW)(GSM_SIZE);
  memset(m, 0, GSM_SIZE);
  FN(fn_ctor, OFF_GSM_CTOR)(m, 1, ctx_a, ctx_b, 0);
  FN(fn_v_pi, OFF_GSM_SETUP)(m, 1);
  // 16 版新增：注册监听器（照抄 NativeReplayRunner 0xd9ae8c..0xd9aed0）
  uint64_t *lis = FN(fn_p_sz, OFF_NEW)(0x20);
  memset(lis, 0, 0x20);
  FN(fn_v_p, OFF_LISTENER_INIT)(lis);
  lis[0] = *(uint64_t *)(g_libg + OFF_LISTENER_VT_GOT) + 0x10;
  lis[1] = 0;
  lis[2] = 0;
  lis[3] = 0x4000000000ull;
  void *lis_ref = lis;
  FN(fn_v_pp, OFF_LISTENER_ADD)((void *)RP((uintptr_t)m + 0x40), &lis_ref);
  g->m = m;
  int ok = FN(fn_load, OFF_GSM_LOAD)(m, g->root, NULL, NULL);
  uintptr_t world = RP((uintptr_t)m + 0xa8);
  LOGI("engine: manager=%p load=%d world=%p", m, ok & 1, (void *)world);
  return okp(world);
}

static void engine_step_once(struct Engine *g) {
  void (*step)(void *) = g_orig_step ? g_orig_step : FN(fn_v_p, 0x11d3174);
  step(g->m);
}

static int32_t engine_tick(struct Engine *g) { return FN(fn_i_p, OFF_GSM_TICK)(g->m); }

// 推进 n 个 tick。返回 1 = 对局已结束（连续 8 次 step 时钟都不走）。
static int engine_advance(struct Engine *g, int n) {
  int32_t start = engine_tick(g), prev = start, stagnant = 0;
  while (prev - start < n) {
    engine_step_once(g);
    int32_t t = engine_tick(g);
    if (t == prev) {
      if (++stagnant >= 8) return 1;
      continue;
    }
    stagnant = 0;
    prev = t;
  }
  return 0;
}

// 把一条指令 JSON（{"ct":153,"c":{...}} 或技能 {"ct":189,...}）放进指令队列（管理器 +0x38）。
// 指令上下文：16 版为 *(管理器+0x210)（取值函数 0x11d48a4；15 版 +0x208）。解析函数用 x8 返回结果，经汇编桥调用。
#define OFF_CMD_PARSE 0xe553e4
#define OFF_CMD_APPEND 0xe5443c
extern void cr_call_logic_command_parse(void *json_object, void *command_context, void **result, void *target);

static int engine_inject(struct Engine *g, const char *text, long len) {
  void *obj = parse_json(text, len);
  if (!obj) return 0;
  void *ctx = (void *)RP((uintptr_t)g->m + 0x210);
  void *queue = (void *)RP((uintptr_t)g->m + 0x38);
  void *cmd = NULL;
  if (ctx && queue) cr_call_logic_command_parse(obj, ctx, &cmd, (void *)(g_libg + OFF_CMD_PARSE));
  release_json(obj);
  if (!cmd) return 0;
  FN(fn_v_pp, OFF_CMD_APPEND)(queue, &cmd);   // append 接管指令对象
  return 1;
}

// 旧接口：整局指令都在 JSON 里，一口气跑完并逐帧写文件（批量重演用）。
static void headless_run(char *json, long len) {
  LOGI("headless: json %ld bytes", len);
  pthread_mutex_lock(&g_engine_mu);
  struct Engine g;
  if (engine_create(&g, json, len)) {
    FILE *o = fopen(DUMP_DIR "/crprobe_out.jsonl", "w");
    if (o) {
      int32_t prev = engine_tick(&g);
      emit_frame(o, prev, RP((uintptr_t)g.m + 0xa8));
      for (long steps = 0; steps < 40000; steps++) {
        if (engine_advance(&g, 1)) break;
        uintptr_t world = RP((uintptr_t)g.m + 0xa8);
        if (!okp(world)) break;
        emit_frame(o, engine_tick(&g), world);
      }
      fclose(o);
    }
  }
  LOGI("headless: finished last_tick=%d", g.m ? engine_tick(&g) : -1);
  engine_destroy(&g);   // 也会 free(json)
  pthread_mutex_unlock(&g_engine_mu);
  LOGI("headless: cleaned up, rss=%ld kB", rss_kb());
}

// ---------- 交互式环境：TCP 127.0.0.1:26789（主机 adb forward tcp:26789 tcp:26789） ----------
// 协议（每条请求一行头，可带定长正文；每条回复一行 JSON）：
//   RESET <len>\n<开局 JSON>   新建一局（销毁上一局），回复当前帧
//   CMD <len>\n<指令 JSON>     放进指令队列，回复 {"ok":1|0}
//   STEP <n>\n                 推进 n 个 tick，回复当前帧（多 "ended" 字段）
//   CLOSE\n                    销毁当前对局
#include <netinet/in.h>
#include <sys/socket.h>
#define ENV_PORT 26789

static int read_line(int fd, char *buf, int cap) {
  int n = 0;
  while (n < cap - 1) {
    char ch;
    ssize_t r = recv(fd, &ch, 1, 0);
    if (r <= 0) return -1;
    if (ch == '\n') break;
    buf[n++] = ch;
  }
  buf[n] = 0;
  return n;
}

static char *read_body(int fd, long len) {
  char *b = malloc((size_t)len + 1);
  long got = 0;
  while (b && got < len) {
    ssize_t r = recv(fd, b + got, (size_t)(len - got), 0);
    if (r <= 0) { free(b); return NULL; }
    got += r;
  }
  if (b) b[len] = 0;
  return b;
}

static void send_all(int fd, const char *p, size_t n) {
  while (n) {
    ssize_t w = send(fd, p, n, 0);
    if (w <= 0) return;
    p += w;
    n -= (size_t)w;
  }
}

static void send_frame(int fd, struct Engine *g, int ended) {
  char *buf = NULL;
  size_t sz = 0;
  FILE *o = open_memstream(&buf, &sz);
  uintptr_t world = g->m ? RP((uintptr_t)g->m + 0xa8) : 0;
  int have = okp(world);
  if (have) emit_frame(o, engine_tick(g), world);   // 以 "}\n" 结尾
  fclose(o);
  if (have && sz >= 2) {
    send_all(fd, buf, sz - 2);                       // 去掉末尾 "}\n"，补上 ended 字段
    char tail[32];
    int k = snprintf(tail, sizeof tail, ",\"ended\":%d}\n", ended);
    send_all(fd, tail, (size_t)k);
  } else {
    send_all(fd, "{\"error\":\"no battle\"}\n", 22);
  }
  free(buf);
}

// 多客户端（2026-11-21）：每个连接一个线程、各自一局（各自的管理器），这样多个采集进程可以并行打局。
// 引擎调用仍由 g_engine_mu 串行化（单次调用只要几毫秒）；FirstLight 也是一个进程里同时驻留多局。
// 连接断开时销毁该连接的对局（客户端崩溃也不会留下垃圾对局占内存）。
static int g_clients = 0;
#define MAX_CLIENTS 24

static void *env_client(void *arg) {
  int c = (int)(intptr_t)arg;
  int one = 1;
  setsockopt(c, IPPROTO_TCP, 1 /* TCP_NODELAY */, &one, sizeof one);
  struct Engine g = {0};
  {
    char line[128];
    while (read_line(c, line, sizeof line) >= 0) {
      long len = 0;
      int n = 0;
      if (sscanf(line, "RESET %ld", &len) == 1) {
        char *body = read_body(c, len);
        if (!body) break;
        pthread_mutex_lock(&g_engine_mu);
        if (g.m || g.json) engine_destroy(&g);
        int ok = engine_create(&g, body, len);
        if (!ok) engine_destroy(&g);
        send_frame(c, &g, 0);
        pthread_mutex_unlock(&g_engine_mu);
      } else if (sscanf(line, "CMD %ld", &len) == 1) {
        char *body = read_body(c, len);
        if (!body) break;
        pthread_mutex_lock(&g_engine_mu);
        int ok = g.m ? engine_inject(&g, body, len) : 0;
        pthread_mutex_unlock(&g_engine_mu);
        free(body);
        char r[32];
        int k = snprintf(r, sizeof r, "{\"ok\":%d}\n", ok);
        send_all(c, r, (size_t)k);
      } else if (sscanf(line, "STEP %d", &n) == 1) {
        pthread_mutex_lock(&g_engine_mu);
        int ended = g.m ? engine_advance(&g, n) : 1;
        send_frame(c, &g, ended);
        pthread_mutex_unlock(&g_engine_mu);
      } else if (!strncmp(line, "LIVE ", 5)) {
        // 实时对局：外部读取器定位到的世界对象地址（读取器帧里的 roster）。输出格式与离线引擎相同，
        // tick = *(*(世界+0x8)+0x60)。只读内存，另外调用 CardSelection::build 计算手牌 pd。
        uintptr_t world = (uintptr_t)strtoull(line + 5, NULL, 16);
        uintptr_t ctx = 0;
        int32_t tk = -1;
        int ok = okp(world) && safe_read(world + 0x8, &ctx, 8) && okp(ctx) && safe_read(ctx + 0x60, &tk, 4);
        uintptr_t k0 = 0;
        ok = ok && safe_read(world + 0xe0, &k0, 8) && okp(k0) && RP(k0) == g_libg + OFF_KING_VT;
        if (!ok) {
          send_all(c, "{\"error\":\"no live battle\"}\n", 27);
        } else {
          char *buf = NULL;
          size_t sz = 0;
          FILE *o = open_memstream(&buf, &sz);
          emit_frame(o, tk, world);
          fclose(o);
          send_all(c, buf, sz);
          free(buf);
        }
      } else if (!strncmp(line, "WDUMP", 5)) {
        // 调试：当前离线对局的 tick 和世界对象前 0x800 字节（按 int32），用来在世界对象里找 tick 字段
        pthread_mutex_lock(&g_engine_mu);
        uintptr_t world = g.m ? RP((uintptr_t)g.m + 0xa8) : 0;
        int32_t w[0x200] = {0};
        if (okp(world)) safe_read(world, w, sizeof w);
        char *buf = NULL;
        size_t sz = 0;
        FILE *o = open_memstream(&buf, &sz);
        int32_t tk = g.m ? engine_tick(&g) : -1;
        fprintf(o, "{\"tick\":%d,\"w\":[", tk);
        for (int k = 0; k < 0x200; k++) fprintf(o, "%s%d", k ? "," : "", w[k]);
        // 顺着世界对象里的指针再找一层：[世界内偏移, 子对象内偏移]，子对象里值等于 tick 的字段
        fprintf(o, "],\"hits\":[");
        int fh = 1;
        for (int k = 0; k < 0x200 / 2; k++) {
          uintptr_t p = ((uintptr_t *)w)[k];
          int32_t sub[0x100];
          if (!okp(p) || !safe_read(p, sub, sizeof sub)) continue;
          for (int m = 0; m < 0x100; m++)
            if (sub[m] == tk) { fprintf(o, "%s[%d,%d]", fh ? "" : ",", k * 8, m * 4); fh = 0; }
        }
        fprintf(o, "]}\n");
        fclose(o);
        pthread_mutex_unlock(&g_engine_mu);
        send_all(c, buf, sz);
        free(buf);
      } else if (!strncmp(line, "WADDR", 5)) {
        // 调试：当前离线对局的世界对象地址
        pthread_mutex_lock(&g_engine_mu);
        uintptr_t world = g.m ? RP((uintptr_t)g.m + 0xa8) : 0;
        pthread_mutex_unlock(&g_engine_mu);
        char r[64];
        int k = snprintf(r, sizeof r, "{\"world\":\"%lx\",\"libg\":\"%lx\"}\n", (unsigned long)world, (unsigned long)g_libg);
        send_all(c, r, (size_t)k);
      } else if (!strncmp(line, "PEEK ", 5)) {
        // 调试：只读内存 PEEK <十六进制地址> <字节数 ≤ 4096>，回复 {"hex":"..."}（读不了时 hex 为空）
        unsigned long addr = 0;
        int len = 0;
        sscanf(line + 5, "%lx %d", &addr, &len);
        if (len < 0) len = 0;
        if (len > 4096) len = 4096;
        uint8_t mem[4096];
        int okr = len > 0 && safe_read((uintptr_t)addr, mem, (size_t)len);
        char *out = malloc(2 * (size_t)len + 32);
        int k = sprintf(out, "{\"hex\":\"");
        for (int q = 0; okr && q < len; q++) k += sprintf(out + k, "%02x", mem[q]);
        k += sprintf(out + k, "\"}\n");
        send_all(c, out, (size_t)k);
        free(out);
      } else if (!strncmp(line, "CLOSE", 5)) {
        pthread_mutex_lock(&g_engine_mu);
        engine_destroy(&g);
        pthread_mutex_unlock(&g_engine_mu);
        send_all(c, "{\"ok\":1}\n", 9);
      } else {
        send_all(c, "{\"error\":\"bad request\"}\n", 24);
      }
    }
  }
  pthread_mutex_lock(&g_engine_mu);
  if (g.m || g.json) engine_destroy(&g);
  pthread_mutex_unlock(&g_engine_mu);
  close(c);
  __atomic_sub_fetch(&g_clients, 1, __ATOMIC_RELAXED);
  return NULL;
}

static void *env_server(void *arg) {
  (void)arg;
  while (__atomic_load_n(&g_step_calls, __ATOMIC_RELAXED) < 100) usleep(200 * 1000);
  int s = socket(AF_INET, SOCK_STREAM, 0);
  int one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in a = {0};
  a.sin_family = AF_INET;
  a.sin_port = htons(ENV_PORT);
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(s, (struct sockaddr *)&a, sizeof a) != 0 || listen(s, MAX_CLIENTS) != 0) {
    LOGI("env: bind/listen failed");
    return NULL;
  }
  LOGI("env: listening on %d (multi-client, max %d)", ENV_PORT, MAX_CLIENTS);
  for (;;) {
    int c = accept(s, NULL, NULL);
    if (c < 0) continue;
    if (__atomic_add_fetch(&g_clients, 1, __ATOMIC_RELAXED) > MAX_CLIENTS) {
      send_all(c, "{\"error\":\"too many clients\"}\n", 30);
      close(c);
      __atomic_sub_fetch(&g_clients, 1, __ATOMIC_RELAXED);
      continue;
    }
    pthread_t th;
    pthread_attr_t at;
    pthread_attr_init(&at);
    pthread_attr_setdetachstate(&at, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&at, 1 << 20);
    if (pthread_create(&th, &at, env_client, (void *)(intptr_t)c) != 0) {
      close(c);
      __atomic_sub_fetch(&g_clients, 1, __ATOMIC_RELAXED);
    }
    pthread_attr_destroy(&at);
  }
  return NULL;
}

static void *headless_thread(void *arg) {
  (void)arg;
  // 启动时的残留任务不处理；等主界面跑起来（Step 被调用过 100 次以上，资源已加载）再接任务，
  // 否则游戏资源没加载完就跑引擎会崩溃。
  unlink(DUMP_DIR "/crprobe_in.json");
  while (__atomic_load_n(&g_step_calls, __ATOMIC_RELAXED) < 100) usleep(200 * 1000);
  LOGI("headless: ready");
  for (;;) {
    long len = 0;
    char *json = read_file(DUMP_DIR "/crprobe_in.json", &len);
    if (json) {
      unlink(DUMP_DIR "/crprobe_in.json");
      unlink(DUMP_DIR "/crprobe_done");
      headless_run(json, len);  // 接管 json 的所有权
      FILE *d = fopen(DUMP_DIR "/crprobe_done", "w");
      if (d) fclose(d);
    }
    usleep(300 * 1000);
  }
  return NULL;
}

struct Hook {
  const char *name;
  uintptr_t offset;
  uint8_t prologue[16];
  void *replacement;
  void **original;
};

static struct Hook hooks[] = {
    {"GameStateStep", 0x11d3174,
     {0xfd, 0x7b, 0xbd, 0xa9, 0xf5, 0x0b, 0x00, 0xf9, 0xf4, 0x4f, 0x02, 0xa9, 0xfd, 0x03, 0x00, 0x91},
     (void *)step_hook, (void **)&g_orig_step},
    {"ReplayJsonSetter", 0xda1f90,
     {0xfd, 0x7b, 0xbe, 0xa9, 0xf4, 0x4f, 0x01, 0xa9, 0xfd, 0x03, 0x00, 0x91, 0xf4, 0x03, 0x01, 0xaa},
     (void *)replay_setter_hook, (void **)&g_orig_replay_setter},
};

// ---------- 安装 ----------
// ldr x17, #8 ; br x17 ; .quad target
static void emit_jump(uint8_t *at, uintptr_t target) {
  const uint32_t ldr = 0x58000051u, br = 0xd61f0220u;
  memcpy(at, &ldr, 4);
  memcpy(at + 4, &br, 4);
  memcpy(at + 8, &target, 8);
}

static int set_prot(void *addr, size_t len, int prot) {
  long page = sysconf(_SC_PAGESIZE);
  uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(page - 1);
  uintptr_t end = ((uintptr_t)addr + len + page - 1) & ~(uintptr_t)(page - 1);
  return mprotect((void *)start, end - start, prot);
}

static int install_hook(struct Hook *h) {
  uint8_t *target = (uint8_t *)(g_libg + h->offset);
  if (memcmp(target, h->prologue, 16) != 0) {
    LOGI("%s: prologue mismatch, skip", h->name);
    return 0;
  }
  uint8_t *tramp = mmap(NULL, 32, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (tramp == MAP_FAILED) return 0;
  memcpy(tramp, target, 16);
  emit_jump(tramp + 16, (uintptr_t)(target + 16));
  __builtin___clear_cache((char *)tramp, (char *)tramp + 32);
  *h->original = tramp;
  if (set_prot(target, 16, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
    LOGI("%s: mprotect failed", h->name);
    return 0;
  }
  emit_jump(target, (uintptr_t)h->replacement);
  __builtin___clear_cache((char *)target, (char *)target + 16);
  set_prot(target, 16, PROT_READ | PROT_EXEC);
  LOGI("%s hook installed at %p", h->name, target);
  return 1;
}

// ---------- 找 libg ----------
// 转译环境下 dl_iterate_phdr 看不到 libg，改读 /proc/self/maps。进程里可能有不止一份 libg.so 映射
// （例如崩溃上报 SDK 为读符号把文件映射进来），认错基址会导致钩子全部装不上（2026-11-21 遇到过）。
// 验证方法：候选基址 + GameStateStep 偏移必须落在同一文件、同一装载基址的某段可读映射里，再比对开头 16 字节。
#define CHECK_OFF 0x11d3174
static const uint8_t kCheckPrologue[16] = {0xfd, 0x7b, 0xbd, 0xa9, 0xf5, 0x0b, 0x00, 0xf9,
                                           0xf4, 0x4f, 0x02, 0xa9, 0xfd, 0x03, 0x00, 0x91};

struct MapSeg { unsigned long start, end, off; char r, w; };

static uintptr_t find_libg_maps(void) {
  FILE *f = fopen("/proc/self/maps", "r");
  if (!f) return 0;
  struct MapSeg seg[64];
  int n = 0;
  char line[512];
  while (fgets(line, sizeof line, f) && n < 64) {
    unsigned long start, end, off;
    char perms[8];
    if (sscanf(line, "%lx-%lx %7s %lx", &start, &end, perms, &off) == 4 && strstr(line, "/libg.so"))
      seg[n++] = (struct MapSeg){start, end, off, perms[0], perms[1]};
  }
  fclose(f);
  for (int i = 0; i < n; i++) {
    if (seg[i].off != 0) continue;
    uintptr_t base = seg[i].start, target = base + CHECK_OFF;
    // 真正由链接器加载的 libg 在其后不远处有可写的数据段（rw-p）；崩溃上报 SDK 映射进来读符号的只读副本没有。
    // 数据段的虚拟地址比文件偏移大 0x8000，所以不能用 start - off == base 判断，只看位置范围。
    int loaded = 0;
    for (int j = 0; j < n; j++)
      if (seg[j].w == 'w' && seg[j].start > base && seg[j].start < base + 0x2000000) loaded = 1;
    if (!loaded) continue;
    for (int j = 0; j < n; j++) {
      // 同一装载基址：start - off == base；目标地址在这段里且可读
      if (seg[j].start - seg[j].off == base && seg[j].r == 'r' && target >= seg[j].start && target + 16 <= seg[j].end) {
        if (memcmp((void *)target, kCheckPrologue, 16) == 0) return base;
      }
    }
  }
  return 0;
}

// 本库可能先于 libg 被加载（Java 层的 sentry 先 loadLibrary），所以后台线程等 libg 映射出现再装钩子。
static void *wait_thread(void *arg) {
  (void)arg;
  for (int i = 0; i < 600 && !g_libg; i++) {
    g_libg = find_libg_maps();   // 不用 dl_iterate_phdr：可能返回未经验证的其他映射
    if (!g_libg) usleep(100 * 1000);
  }
  LOGI("libg=%p", (void *)g_libg);
  if (!g_libg) return NULL;
  usleep(200 * 1000);  // 等 libg 重定位完成
  for (size_t i = 0; i < sizeof hooks / sizeof hooks[0]; i++) install_hook(&hooks[i]);
  pthread_t env;
  pthread_create(&env, NULL, env_server, NULL);
  pthread_detach(env);
  headless_thread(NULL);
  return NULL;
}

__attribute__((constructor)) static void probe_init(void) {
  LOGI("probe loaded pid=%d", getpid());
  pthread_t t;
  pthread_create(&t, NULL, wait_thread, NULL);
  pthread_detach(t);
}
