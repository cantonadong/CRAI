"""模拟器设备选择：优先环境变量 CRAI_SERIAL，否则自动选 adb devices 中第一个在线设备
（LDPlayer 重启后可能从 127.0.0.1:5555 变成 emulator-5554）。"""

from __future__ import annotations

import os
import subprocess

ADB = r"D:\Android\sdk\platform-tools\adb.exe"


def instance_of(serial: str) -> int:
    """adb 设备名 → LDPlayer 实例编号：emulator-5554 / 127.0.0.1:5555 → 0，emulator-5556 / 127.0.0.1:5557 → 1 …"""
    tail = serial.rsplit("-", 1)[-1].rsplit(":", 1)[-1]
    if not tail.isdigit():
        return 0
    n = int(tail)
    n = n - 1 if n % 2 else n
    return max(0, (n - 5554) // 2)


def _online() -> list[str]:
    out = subprocess.run([ADB, "devices"], capture_output=True, text=True).stdout
    return [l.split("\t")[0] for l in out.splitlines()[1:] if l.endswith("\tdevice")]


def detect_serial() -> str:
    """多实例约定（2026-11-21）：实例 0 = 天梯/实战（默认），实例 1 = 离线引擎（重演、PPO）。
    要用其他实例必须显式设置 CRAI_SERIAL（例如 emulator-5556），或 CRAI_INSTANCE=1。
    以前是"取第一个在线设备"，多实例时可能连错（最坏情况在天梯号上跑引擎任务），已改掉。"""
    env = os.environ.get("CRAI_SERIAL")
    if env:
        return env
    want = int(os.environ.get("CRAI_INSTANCE", "0"))
    devs = _online()
    if not any(instance_of(d) == want for d in devs) and want == 0:
        subprocess.run([ADB, "connect", "127.0.0.1:5555"], capture_output=True)
        devs = _online()
    # 同一实例可能同时出现 emulator-555x 与 127.0.0.1:555x+1，取 emulator- 那个
    mine = sorted((d for d in devs if instance_of(d) == want), key=lambda d: not d.startswith("emulator-"))
    return mine[0] if mine else f"emulator-{5554 + 2 * want}"


SERIAL = detect_serial()


def adb_is_root() -> bool:
    """adbd 是否已以 root 运行。注意：不要主动 `adb root`——LDPlayer 上 root adbd 的每个 shell 都会弹
    "已授予 Shell 超级用户权限"提示，比 su 更频繁（2026-11-21 实测）。"""
    try:
        out = subprocess.run([ADB, "-s", SERIAL, "shell", "id -u"], capture_output=True, text=True, timeout=15).stdout
    except subprocess.TimeoutExpired:
        return False
    return out.strip() == "0"


ROOT_ADB = adb_is_root()


def as_root(cmd: str) -> str:
    """需要 root 的 shell 命令统一从这里包装：adbd 已是 root 时原样执行，否则用 su -c。"""
    if ROOT_ADB:
        return cmd
    return "su -c '" + cmd.replace("'", "'\\''") + "'"
