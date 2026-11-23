// 常驻触摸注入：直接向模拟器触摸屏设备（多点触控 Protocol B）写事件，代替 `adb shell input`
// （后者每次启动一个 Java 进程，一次点击约 140ms）。走的仍是系统正常的触摸输入路径。
// 从 stdin 逐行读命令（屏幕坐标，竖屏 1080x1920）：
//   T x y            点击（按下 30ms 后抬起）
//   W x1 y1 x2 y2 ms 拖拽（ms 毫秒内移动，约每 8ms 一个点）
//   P                回复 "ok"（用来测往返延迟）
// 每条命令执行完回复一行 "ok"。用法：touchd <设备路径> <坐标映射 0..3>
// 坐标映射：设备是横屏 1920x1080，映射 m：0=(sy, sx) 1=(sy, 1079-sx) 2=(1919-sy, sx) 3=(1919-sy, 1079-sx)
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static int fd, map;
static int tracking = 100;

static void ev(int type, int code, int value) {
  struct input_event e;
  memset(&e, 0, sizeof e);
  gettimeofday(&e.time, NULL);
  e.type = type;
  e.code = code;
  e.value = value;
  write(fd, &e, sizeof e);
}

static void to_dev(int sx, int sy, int *dx, int *dy) {
  int a, b;
  if (map == 4) {
    // LDPlayer 实测（2026-11-21，dumpsys input 对照）：系统把设备 X 0..1919 缩放到屏幕 X 0..1080，
    // 设备 Y 0..1079 缩放到屏幕 Y 0..1920，不旋转。所以设备坐标 = 屏幕坐标按轴反向缩放。
    a = (sx * 1920 + 540) / 1080;
    b = (sy * 1080 + 960) / 1920;
  } else {
    a = sy; b = sx;                          // 旧的试验映射（0..3），保留做对照
    if (map & 2) a = 1919 - a;
    if (map & 1) b = 1079 - b;
  }
  *dx = a < 0 ? 0 : (a > 1919 ? 1919 : a);
  *dy = b < 0 ? 0 : (b > 1079 ? 1079 : b);
}

static void down(int sx, int sy) {
  int x, y;
  to_dev(sx, sy, &x, &y);
  ev(EV_ABS, ABS_MT_SLOT, 0);
  ev(EV_ABS, ABS_MT_TRACKING_ID, tracking++);
  ev(EV_ABS, ABS_MT_POSITION_X, x);
  ev(EV_ABS, ABS_MT_POSITION_Y, y);
  ev(EV_ABS, ABS_MT_PRESSURE, 1);
  ev(EV_KEY, BTN_TOUCH, 1);
  ev(EV_SYN, SYN_REPORT, 0);
}

static void move(int sx, int sy) {
  int x, y;
  to_dev(sx, sy, &x, &y);
  ev(EV_ABS, ABS_MT_SLOT, 0);
  ev(EV_ABS, ABS_MT_POSITION_X, x);
  ev(EV_ABS, ABS_MT_POSITION_Y, y);
  ev(EV_SYN, SYN_REPORT, 0);
}

static void up(void) {
  ev(EV_ABS, ABS_MT_SLOT, 0);
  ev(EV_ABS, ABS_MT_TRACKING_ID, -1);
  ev(EV_KEY, BTN_TOUCH, 0);
  ev(EV_SYN, SYN_REPORT, 0);
}

int main(int argc, char **argv) {
  const char *path = argc > 1 ? argv[1] : "/dev/input/event2";
  map = argc > 2 ? argv[2][0] - '0' : 4;
  fd = open(path, O_WRONLY);
  if (fd < 0) { perror("open"); return 1; }
  setvbuf(stdout, NULL, _IOLBF, 0);
  char line[128];
  while (fgets(line, sizeof line, stdin)) {
    int x1, y1, x2, y2, ms;
    if (sscanf(line, "T %d %d", &x1, &y1) == 2) {
      down(x1, y1);
      usleep(30000);
      up();
    } else if (sscanf(line, "W %d %d %d %d %d", &x1, &y1, &x2, &y2, &ms) == 5) {
      int steps = ms / 8 < 2 ? 2 : ms / 8;
      down(x1, y1);
      for (int i = 1; i <= steps; i++) {
        usleep(8000);
        move(x1 + (x2 - x1) * i / steps, y1 + (y2 - y1) * i / steps);
      }
      usleep(20000);
      up();
    }
    printf("ok\n");
  }
  return 0;
}
