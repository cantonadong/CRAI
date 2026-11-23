"""交互式离线对局环境（客户端）：连接游戏进程内代理库（crai/probe/proxy_min.c）的 TCP 服务。

    env = NativeEnv()
    frame = env.reset(native_json)        # 开局 JSON（游戏原生格式，cmd 通常为空）
    env.inject(cmd)                       # 一条原生指令：出牌 ct=153 / 技能 ct=189
    frame = env.step(5)                   # 推进 5 tick，返回当前帧；frame["ended"] = 1 表示对局结束

帧格式与离线重演输出相同：tick、players（side/elixir/seq/hand/next/deck/king_hp）、entities（gid/kind/side/x/y/card/hp…）。
"""

from __future__ import annotations

import json
import socket
import subprocess
import time

from crai.device import ADB, SERIAL

PORT = 26789          # 游戏进程内（设备端）监听的端口
COMMAND_AGE = 20      # 下达到生效的 tick 数（实战同样是 20）


def host_port(serial: str = SERIAL) -> int:
    """主机端转发端口：多个模拟器实例同时用时不能都占 26789。
    实例 0（emulator-5554 / 127.0.0.1:5555）用 26789，其他实例按 adb 端口错开（emulator-5556 → 26790）。
    可用环境变量 CRAI_ENV_PORT 直接指定。"""
    import os
    if os.environ.get("CRAI_ENV_PORT"):
        return int(os.environ["CRAI_ENV_PORT"])
    tail = serial.rsplit("-", 1)[-1].rsplit(":", 1)[-1]
    if tail.isdigit():
        n = int(tail)
        n = n - 1 if n % 2 else n          # 127.0.0.1:5557 与 emulator-5556 是同一实例
        return PORT + max(0, (n - 5554) // 2)
    return PORT


class NativeEnv:
    def __init__(self, port: int = PORT, timeout: float = 60.0, wait: float = 120.0):
        """游戏刚启动时服务要等主界面跑起来才监听；adb 端口转发会先接受连接再立刻断开，所以用 CLOSE 探测，没就绪就重试。
        port 是设备端端口；主机端端口见 host_port()。"""
        hport = host_port()
        subprocess.run([ADB, "-s", SERIAL, "forward", f"tcp:{hport}", f"tcp:{port}"], capture_output=True, timeout=30)
        port = hport
        deadline = time.time() + wait
        while True:
            try:
                self.sock = socket.create_connection(("127.0.0.1", port), timeout=timeout)
                self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                self.rf = self.sock.makefile("rb")
                self._request("CLOSE")
                return
            except (OSError, ConnectionError, ValueError):
                try:
                    self.sock.close()
                except Exception:  # noqa: BLE001
                    pass
                if time.time() > deadline:
                    raise ConnectionError("离线环境服务未就绪（游戏没在运行或代理库没加载）")
                time.sleep(2)

    def _request(self, head: str, body: bytes | None = None) -> dict:
        data = head.encode() + b"\n" + (body or b"")
        self.sock.sendall(data)
        line = self.rf.readline()
        if not line:
            raise ConnectionError("离线环境连接断开（游戏可能崩溃）")
        return json.loads(line)

    def reset(self, native: dict) -> dict:
        body = json.dumps(native, ensure_ascii=False).encode("utf-8")
        f = self._request(f"RESET {len(body)}", body)
        if "error" in f:
            raise RuntimeError(f"开局失败：{f['error']}")
        return f

    def inject(self, cmd: dict) -> bool:
        body = json.dumps(cmd).encode()
        return bool(self._request(f"CMD {len(body)}", body).get("ok"))

    def step(self, ticks: int = 1) -> dict:
        return self._request(f"STEP {int(ticks)}")

    def live(self, world: str | int) -> dict | None:
        """实时对局的当前帧（world = 实时读取器帧里的 roster 地址）；不在对局中返回 None。"""
        w = world if isinstance(world, str) else f"{world:x}"
        f = self._request(f"LIVE {w}")
        return None if "error" in f else f

    def peek(self, addr: int, n: int) -> bytes:
        """调试：读游戏进程内存（代理库 PEEK，2026-11-22 起）。读不了返回空。"""
        return bytes.fromhex(self._request(f"PEEK {addr:x} {int(n)}")["hex"])

    def world_addr(self) -> int:
        """调试：当前离线对局的世界对象地址。"""
        return int(self._request("WADDR")["world"], 16)

    def close(self):
        try:
            self._request("CLOSE")
        except Exception:  # noqa: BLE001
            pass
        self.sock.close()


def play_cmd(tick: int, account_lo: int, card_id: int, pd: int, x: int, y: int) -> dict:
    """在当前 tick 下达的出牌指令（t = 当前 tick，t2 = t + 20 生效）。pd 见 scripts/royaleapi_to_native.py: PdEncoder。"""
    return {"ct": 153, "c": {"t": tick, "t2": tick + COMMAND_AGE, "idHi": 0, "idLo": account_lo, "px": x, "py": y,
                             "sid": -1, "sel": {"os": card_id, "pd": pd}}}


def ability_cmd(tick: int, account_lo: int, gid: int) -> dict:
    return {"ct": 189, "c": {"t": tick, "t2": tick + COMMAND_AGE, "idHi": 0, "idLo": account_lo, "cgid": gid}}
