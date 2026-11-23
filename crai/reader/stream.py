"""Python 端：在模拟器里启动 cr_reader，逐行解析 JSON 战局帧。"""

from __future__ import annotations

import json
import subprocess
import time
from dataclasses import dataclass, field
from typing import Iterator

from crai.device import ADB, SERIAL, as_root
PACKAGE = "nullsroyale.rel.free"
READER = "/data/local/tmp/cr_reader"


def _load_own_deck() -> set[int]:
    from pathlib import Path
    p = Path(__file__).resolve().parents[2] / "configs" / "own_deck.json"
    try:
        return set(json.loads(p.read_text(encoding="utf-8"))["cards"])
    except (OSError, KeyError, ValueError):
        return set()


OWN_DECK = _load_own_deck()


@dataclass
class Player:
    side: int
    elixir: float
    seq: int
    king_hp: tuple[int, int]
    hand: list[int]        # deck index
    next: list[int]        # deck index 队列
    deck: list[int]        # deck index -> card id
    last_card: int = -1    # 最近打出的卡牌 ID

    def hand_cards(self) -> list[int]:
        return [self.deck[i] if 0 <= i < 8 else -1 for i in self.hand]

    def next_card(self) -> int:
        i = self.next[0]
        return self.deck[i] if 0 <= i < 8 else -1


@dataclass
class Entity:
    id: str
    kind: int
    side: int
    x: int
    y: int
    card: int
    lvl: int
    hp: int
    max_hp: int
    raw_card: int = -1     # 内存里的原始编号（英雄单位为 203xxxxxx）；card 是换算后的卡牌 ID
    gid: int = 0           # 对象 ID（实体 +0x08；离线引擎输出有，实时读取器暂无）


@dataclass
class Frame:
    t: float
    battle: bool
    players: list[Player] = field(default_factory=list)
    entities: list[Entity] = field(default_factory=list)
    read_ms: float = 0.0
    roster: str = ""  # 对局对象地址；结算界面上旧对局对象仍有效，用它区分新旧对局

    def own_index(self) -> int:
        """己方 player 下标。

        1) 优先按卡组识别：与 configs/own_deck.json 一致的一方（不依赖坐标，可用于验证翻转）
        2) 两边都匹配/都不匹配时，退回"国王塔在内存下方（y>16000）的一方"
        """
        if OWN_DECK:
            match = [i for i, p in enumerate(self.players) if set(p.deck) == OWN_DECK]
            if len(match) == 1:
                return match[0]
        kings = {e.side: e for e in self.entities if e.kind in (12, 13) and e.card == -1 and e.x == 9000}
        for i, p in enumerate(self.players):
            k = kings.get(p.side)
            if k is not None and k.y > 16000:
                return i
        return 1


def base_card_id(cid: int) -> int:
    """进化卡 ID（203xxxxxx，如 203000014 = 进化火枪手）换算为普通卡 ID（26xxxxxx）。"""
    if 203000000 <= cid < 204000000:
        return 26000000 + cid % 1000000
    return cid


_ENTITY_FIELDS = set(Entity.__dataclass_fields__)


def parse_frame(line: str) -> Frame:
    d = json.loads(line)
    f = Frame(t=d["t"], battle=d["battle"], read_ms=d.get("read_ms", 0.0), roster=d.get("roster", ""))
    for p in d.get("players", []):
        f.players.append(Player(p["side"], p["elixir"], p["seq"], tuple(p["king_hp"]), p["hand"], p["next"], p["deck"],
                                base_card_id(p.get("last_card", -1))))
    for e in d.get("entities", []):
        e["raw_card"] = e["card"]
        e["card"] = base_card_id(e["card"])
        f.entities.append(Entity(**{k: v for k, v in e.items() if k in _ENTITY_FIELDS}))   # 忽略新增字段（dgid 等）
    return f


def _pid() -> str:
    out = subprocess.run([ADB, "-s", SERIAL, "shell", f"pidof {PACKAGE}"], capture_output=True, text=True).stdout.strip()
    if not out:
        raise RuntimeError("游戏未运行")
    return out


def stream(period_ms: int = 33, frames: int = 0, rescan: int = 15) -> Iterator[Frame]:
    cmd = as_root(f"{READER} {_pid()} {period_ms} {frames} {rescan}")
    proc = subprocess.Popen([ADB, "-s", SERIAL, "exec-out", cmd], stdout=subprocess.PIPE, text=True, bufsize=1)
    try:
        for line in proc.stdout:
            line = line.strip()
            if line.startswith("{"):
                yield parse_frame(line)
    finally:
        proc.kill()
        # 关掉 adb 客户端不会结束模拟器里的读取器进程，显式清理
        subprocess.run([ADB, "-s", SERIAL, "shell", as_root("pkill cr_reader")], capture_output=True)


class LiveReader:
    """后台线程持续消费读取器输出，只保留最新一帧，避免处理慢时读到积压的旧帧。"""

    def __init__(self, period_ms: int = 30, rescan: int = 5):
        import threading
        self._latest: Frame | None = None
        self._n = 0
        self._cv = threading.Condition()
        self._stop = False
        self._proc = None
        self._th = threading.Thread(target=self._run, args=(period_ms, rescan), daemon=True)
        self._th.start()

    def _run(self, period_ms, rescan):
        cmd = as_root(f"{READER} {_pid()} {period_ms} 0 {rescan}")
        self._proc = subprocess.Popen([ADB, "-s", SERIAL, "exec-out", cmd], stdout=subprocess.PIPE, text=True, bufsize=1)
        for line in self._proc.stdout:
            if self._stop:
                break
            line = line.strip()
            if not line.startswith("{"):
                continue
            f = parse_frame(line)
            f.arrived = time.time()                       # 到达本机的时间：用来测"读到局面 → 下牌"的延迟
            with self._cv:
                self._latest, self._n = f, self._n + 1
                self._cv.notify_all()

    def latest(self, newer_than: int = -1, timeout: float = 5.0) -> tuple[int, Frame]:
        """返回 (帧序号, 帧)；newer_than 给定时等待比它更新的帧。"""
        with self._cv:
            self._cv.wait_for(lambda: self._latest is not None and self._n > newer_than, timeout=timeout)
            if self._latest is None:
                raise RuntimeError("读取器无输出")
            return self._n, self._latest

    def close(self):
        """结束读取：杀掉 adb 客户端，并清理模拟器内的读取器进程（否则调用方进程可能无法退出）。"""
        self._stop = True
        proc = getattr(self, "_proc", None)
        if proc is not None:
            proc.kill()
        subprocess.run([ADB, "-s", SERIAL, "shell", as_root("pkill cr_reader")], capture_output=True)


def snapshot() -> Frame:
    """读取一帧（含 roster 定位，约 0.2–3 秒）。"""
    for f in stream(frames=1, rescan=0):
        return f
    raise RuntimeError("读取器无输出")


if __name__ == "__main__":
    for fr in stream():
        if not fr.battle:
            print(f"t={fr.t:.2f} 未在对战")
            continue
        me = fr.players[fr.own_index()]
        print(f"t={fr.t:6.2f} 圣水={me.elixir:4.1f} 手牌={me.hand_cards()} 下一张={me.next_card()} "
              f"实体={len(fr.entities)} 读取={fr.read_ms:.1f}ms")
