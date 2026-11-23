"""写内存验证实验：打开对战日志第一局回放 -> 扫描出牌指令 -> 修改一条未来指令的 x（换路）-> 观察单位实际落点。
用法: python -u scripts/replay_poke_test.py
"""
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.device import ADB, SERIAL, as_root  # noqa: E402
from crai.reader.stream import LiveReader, OWN_DECK  # noqa: E402
from ui import go_home, shot, tap  # noqa: E402

PREFER = [27000000, 26000021, 26000038, 26000014]  # 加农炮 > 野猪 > 冰人 > 火枪手


def sh(cmd):
    return subprocess.run([ADB, "-s", SERIAL, "shell", cmd], capture_output=True, text=True).stdout


def main():
    sys.stdout.reconfigure(line_buffering=True)
    assert go_home()
    tap(983, 203); time.sleep(3); tap(647, 130); time.sleep(4)
    tap(900, 917)                    # 第一局"观看"
    t_open = time.time()
    time.sleep(3)
    pid = sh("pidof nullsroyale.rel.free").strip()
    rows = []
    for line in sh(as_root(f"/data/local/tmp/list_cmds {pid}")).splitlines():
        p = line.split()
        if len(p) == 7:
            rows.append(dict(addr=p[0], ta=int(p[1]), tb=int(p[2]), pl=p[3], x=int(p[4]), y=int(p[5]), card=int(p[6])))
    rows.sort(key=lambda r: r["ta"])
    print(f"打开回放 {time.time() - t_open:.1f}s 后扫到 {len(rows)} 条指令，tick 范围 {rows[0]['ta'] if rows else '-'}..{rows[-1]['ta'] if rows else '-'}")
    if not rows:
        return
    own = [r for r in rows if r["card"] in OWN_DECK]
    cand = None
    for c in PREFER:
        cand = next((r for r in own if r["card"] == c and 400 <= r["ta"] <= 1600 and abs(r["x"] - 9000) > 1500), None)
        if cand:
            break
    if not cand:
        print("没有合适的候选指令"); return
    new_x = 18000 - cand["x"]
    addr_x = int(cand["addr"], 16) + 0x28
    print(f"选中: tick {cand['ta']} 卡 {cand['card']} 原落点 ({cand['x']},{cand['y']}) -> 改为 ({new_x},{cand['y']})")
    print(sh(as_root(f"/data/local/tmp/poke {pid} {addr_x:x} {new_x}")).strip())
    # 观察：该卡新实体第一次出现的位置
    live = LiveReader(period_ms=30, rescan=1)
    seen, n, t0 = set(), -1, time.time()
    try:
        while time.time() - t0 < 120:
            n, f = live.latest(newer_than=n, timeout=5)
            if not f.battle:
                continue
            for e in f.entities:
                if e.id in seen:
                    continue
                seen.add(e.id)
                if e.card == cand["card"] and n > 3:
                    d_new = abs(e.x - new_x) + abs(e.y - cand["y"])
                    d_old = abs(e.x - cand["x"]) + abs(e.y - cand["y"])
                    print(f"出现: card {e.card} side {e.side} at ({e.x},{e.y})  距改后位置 {d_new}，距原位置 {d_old}")
                    shot("poke_result")
                    print("结论:", "✅ 修改生效（按改后的指令执行）" if d_new < d_old else "❌ 未生效（仍按原指令）")
                    return
    finally:
        live.close()
    print("120 秒内没观察到该卡出现")


if __name__ == "__main__":
    main()
