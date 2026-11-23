// 屏蔽模拟器里的手柄：LDPlayer 把电脑上的手柄转接成 Android 输入设备 "joystick"，窗口不在前台时也会收到按键，
// 用户在电脑上用手柄玩别的游戏会导致模拟器里误操作。
// 本程序常驻，每 0.5 秒扫描 /dev/input，以独占方式（EVIOCGRAB）打开名字含 joystick/gamepad 的输入设备，
// 之后它的事件只送到本程序，Android 收不到。设备被雷电删除重建（手柄重连、切换窗口等）也会马上重新抓。
// 结束本进程（pkill grab_joystick）即恢复。需要 root。编译：x86_64-linux-android28-clang（与读取器相同）。
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXD 32
static int fds[MAXD];
static dev_t devs[MAXD];
static int n;

static int is_pad(const char *name) {
  return strstr(name, "joystick") || strstr(name, "Joystick") || strstr(name, "gamepad") || strstr(name, "Gamepad");
}

static void scan(void) {
  // 先清理已失效的设备（节点不存在或换了设备号）
  for (int i = 0; i < n;) {
    char name[8];
    if (ioctl(fds[i], EVIOCGVERSION, name) < 0) {
      close(fds[i]);
      fds[i] = fds[n - 1]; devs[i] = devs[n - 1]; n--;
      printf("released stale device\n");
      continue;
    }
    i++;
  }
  DIR *d = opendir("/dev/input");
  if (!d) return;
  struct dirent *ent;
  while ((ent = readdir(d))) {
    if (strncmp(ent->d_name, "event", 5)) continue;
    char path[64], name[128] = "";
    snprintf(path, sizeof path, "/dev/input/%s", ent->d_name);
    struct stat st;
    if (stat(path, &st)) continue;
    int known = 0;
    for (int i = 0; i < n; i++) if (devs[i] == st.st_rdev) known = 1;
    if (known) continue;
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) continue;
    ioctl(fd, EVIOCGNAME(sizeof name), name);
    if (is_pad(name) && n < MAXD) {
      if (ioctl(fd, EVIOCGRAB, 1) == 0) {
        fds[n] = fd; devs[n] = st.st_rdev; n++;
        printf("grabbed %s (%s)\n", path, name);
        fflush(stdout);
        continue;  // 保持打开
      }
      printf("grab failed %s (%s)\n", path, name);
    }
    close(fd);
  }
  closedir(d);
  fflush(stdout);
}

int main(void) {
  struct input_event ev[64];
  for (;;) {
    scan();
    for (int i = 0; i < n; i++)
      while (read(fds[i], ev, sizeof ev) > 0) {}  // 丢弃事件
    usleep(500000);
  }
}
