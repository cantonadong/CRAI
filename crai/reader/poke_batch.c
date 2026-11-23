// 批量写内存（仅用于本机回放注入实验）：文件每行 "地址hex 字节数(4|8) 值(十进制，可负)"。
// 全部写完后逐条读回校验，输出成功/失败条数。
// 用法: poke_batch <pid> <清单文件>
#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/mem", argv[1]);
    int fd = open(path, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    FILE *f = fopen(argv[2], "r");
    if (!f) { perror("list"); return 1; }
    static uint64_t addr[65536]; static int sz[65536]; static long long val[65536];
    int n = 0;
    while (n < 65536 && fscanf(f, "%" SCNx64 " %d %lld", &addr[n], &sz[n], &val[n]) == 3) n++;
    fclose(f);
    for (int i = 0; i < n; i++) {
        if (sz[i] == 8) { uint64_t v = (uint64_t)val[i]; pwrite64(fd, &v, 8, (off64_t)addr[i]); }
        else { int32_t v = (int32_t)val[i]; pwrite64(fd, &v, 4, (off64_t)addr[i]); }
    }
    int ok = 0;
    for (int i = 0; i < n; i++) {
        if (sz[i] == 8) { uint64_t v = 0; pread64(fd, &v, 8, (off64_t)addr[i]); ok += v == (uint64_t)val[i]; }
        else { int32_t v = 0; pread64(fd, &v, 4, (off64_t)addr[i]); ok += v == (int32_t)val[i]; }
    }
    printf("writes=%d verified=%d\n", n, ok);
    return ok == n ? 0 : 2;
}
