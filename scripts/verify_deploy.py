"""验证下牌精度（需在训练营对战中）：按目标格子下单体卡，比较实际落点格子。
同时把结果追加到 re/calib_samples.json（补充后排等区域的标定样本）。
用法: python -u scripts/verify_deploy.py
"""
import json
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from calibrate import CYCLE, HAND_SLOTS_PX, SINGLE_UNIT, _wait, drag  # noqa: E402
from crai.client.control import Screen, deploy, deselect  # noqa: E402

MODE = sys.argv[1] if len(sys.argv) > 1 else "tap"
latencies: list[float] = []
from crai.reader.stream import LiveReader  # noqa: E402

# 目标格子 (tx, ty)：桥前、河边、中路、后排角落、公主塔与国王塔之间
TARGETS = [(3, 17), (14, 17), (8, 18), (0, 20), (17, 20), (6, 22), (11, 22),
           (1, 30), (16, 30), (4, 30), (13, 30), (6, 27), (11, 27), (8, 24), (9, 19)]


def main():
    sys.stdout.reconfigure(line_buffering=True)
    scr = Screen()
    live = LiveReader(period_ms=30, rescan=3)
    results, new_samples, n, t0 = [], [], -1, time.time()
    targets = list(TARGETS)
    try:
        while targets and time.time() - t0 < 180:
            n, f = live.latest(newer_than=n)
            if not f.battle:
                break
            me = f.players[f.own_index()]
            hand = me.hand_cards()
            ch = next((i for i, c in enumerate(hand) if c in SINGLE_UNIT and me.elixir >= SINGLE_UNIT[c] + 0.1), None)
            if ch is None:
                soon = any(c in SINGLE_UNIT and me.elixir + 1.5 >= SINGLE_UNIT[c] for c in hand)
                cyc = next((i for i, c in enumerate(hand) if c in CYCLE and me.elixir >= CYCLE[c][0] + 0.1), None)
                if cyc is not None and not soon:
                    seq0, side = me.seq, me.side
                    drag(*HAND_SLOTS_PX[cyc], *CYCLE[hand[cyc]][1])
                    _wait(live, lambda fr: next(p for p in fr.players if p.side == side).seq > seq0, 3.0)
                time.sleep(0.1)
                continue
            tx, ty = targets.pop(0)
            X, Y = Screen.tile_center(tx, ty)
            card, side = hand[ch], me.side
            known = {e.id for e in f.entities}
            t_dep = time.time()
            px, py = deploy(scr, ch, X, Y, mode=MODE)
            is_new = lambda e: e.card == card and e.side == side and e.id not in known  # noqa: E731
            fr = _wait(live, lambda fr: any(is_new(e) for e in fr.entities), 4.0)
            if fr is None:
                print(f"  目标格({tx},{ty}) 像素({px},{py}) -> 未检测到")
                deselect(); continue
            latencies.append(time.time() - t_dep)
            e = next(e for e in fr.entities if is_new(e))
            ax, ay = e.x // 1000, e.y // 1000
            ok = (ax, ay) == (tx, ty)
            results.append(ok)
            new_samples.append([px, py, e.x, e.y])
            print(f"  目标格({tx},{ty}) 像素({px},{py}) -> 实际格({ax},{ay}) {'✓' if ok else '✗ 偏差(%d,%d)' % (ax - tx, ay - ty)}")
    finally:
        live.close()
    if results:
        latencies.sort()
        print(f"[{MODE}] 命中 {sum(results)}/{len(results)}，下牌->单位出现 延迟中位 {latencies[len(latencies) // 2]:.2f}s")
    path = ROOT / "re" / "calib_samples.json"
    old = json.loads(path.read_text(encoding="utf-8"))
    path.write_text(json.dumps(old + new_samples), encoding="utf-8")
    print(f"已追加 {len(new_samples)} 个样本到 {path.name}")


if __name__ == "__main__":
    main()
