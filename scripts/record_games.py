"""被动录制人类对局（只读内存，不点击）。
- 约 10 帧/秒保存，另外任一方出牌序号变化的帧必存
- 每局一个文件：data/human/<开始时间>_<roster>.jsonl（每行一帧，原始读取器 JSON 字段）
- 新对局判定：对局对象地址变化，或出牌序号回落；对局冻结（读取器 stale 重定位）时自动断开当前文件
一直运行，直到进程被结束。
"""
import json
import sys
import time
from dataclasses import asdict
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.reader.stream import LiveReader  # noqa: E402

OUT = ROOT / "data" / "human"


def main():
    sys.stdout.reconfigure(line_buffering=True)
    OUT.mkdir(parents=True, exist_ok=True)
    live = LiveReader(period_ms=33, rescan=1)
    n = -1
    cur, fh, last_save, last_seq, frames, game_no = None, None, 0.0, None, 0, 0
    last_el, last_el_change = None, time.time()
    print("录制中…（只读内存）")
    last_status, got = time.time(), 0
    try:
        while True:
            n, f = live.latest(newer_than=n, timeout=30)
            got += 1
            if time.time() - last_status > 10:
                last_status = time.time()
                el = [round(p.elixir, 1) for p in f.players] if f.players else []
                print(f"[状态] 帧序号 {n} 本段收到 {got} 帧 battle={f.battle} roster={f.roster} 圣水={el} 当前文件帧数={frames}")
                got = 0
            if not f.battle or not f.players:
                continue
            seq = tuple(p.seq for p in f.players)
            el = tuple(round(p.elixir, 2) for p in f.players)
            now = time.time()
            if el != last_el:
                last_el, last_el_change = el, now
            frozen = now - last_el_change > 4 and not all(e >= 10 for e in el)
            new_game = cur is None or f.roster != cur or (last_seq is not None and sum(seq) < sum(last_seq))
            if frozen and fh:
                print(f"  对局 {game_no} 结束（圣水冻结），共 {frames} 帧"); fh.close(); fh, cur = None, None
                continue
            if frozen:
                continue
            if new_game:
                if fh:
                    print(f"  对局 {game_no} 结束，共 {frames} 帧"); fh.close()
                game_no += 1
                cur, frames = f.roster, 0
                path = OUT / f"{datetime.now():%Y%m%d_%H%M%S}_{f.roster}.jsonl"
                fh = path.open("w", encoding="utf-8")
                print(f"对局 {game_no} 开始 -> {path.name}")
            if seq != last_seq or now - last_save >= 0.1:
                rec = {"wall": round(now, 3), **asdict(f)}
                fh.write(json.dumps(rec, ensure_ascii=False) + "\n")
                fh.flush()
                frames += 1
                last_save = now
            last_seq = seq
    finally:
        if fh:
            fh.close()
        live.close()


if __name__ == "__main__":
    main()
