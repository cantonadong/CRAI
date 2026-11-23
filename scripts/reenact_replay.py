"""在训练营里重演一个 RoyaleAPI 回放中某一方（默认野猪 2.6 一方）的出牌：同样时机、同样位置。
- 回放 tick（20/s）从开局起算；读取器确认开局的时刻作为 0 点
- 该牌在手且圣水够 -> 按时下；不在手/圣水不够 -> 进入等待队列，可出时立即补出（记录延迟）
用法: python -u scripts/reenact_replay.py <回放文件> [side=auto]
"""
import json
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.client.control import Screen, deploy, deselect  # noqa: E402
from crai.data import cards  # noqa: E402
from crai.data.dataset import HOG26, perspective_events  # noqa: E402
from crai.data.replay import load_replay  # noqa: E402
from crai.reader.stream import LiveReader  # noqa: E402
import start_training  # noqa: E402


def main(path, side=None):
    sys.stdout.reconfigure(line_buffering=True)
    r = load_replay(path)
    if side is None:
        side = "team" if set(r.team.card_counts) == set(HOG26) else "opponent"
    name = r.team.name if side == "team" else r.opponent.name
    evs = [e for e in perspective_events(r, side) if e.own and e.x is not None]
    print(f"回放 {Path(path).name}：重演 {name} 的 {len(evs)} 次出牌（{side}）")
    if start_training.main("training") != 0:
        sys.exit("开局失败")
    scr = Screen()
    live = LiveReader(period_ms=30, rescan=1)
    t0 = time.time()
    queue, i, n = [], 0, -1
    log = []
    try:
        while time.time() - t0 < 300 and (i < len(evs) or queue):
            n, f = live.latest(newer_than=n, timeout=5)
            if not f.battle or not f.players:
                break
            now = time.time() - t0
            while i < len(evs) and evs[i].tick / 20 <= now:
                queue.append(evs[i]); i += 1
            if not queue:
                continue
            me = f.players[f.own_index()]
            hand = me.hand_cards()
            e = queue[0]
            cid = cards.card_id(e.slug)
            if cid in hand and me.elixir >= cards.cost(cid):
                seq0 = me.seq
                deploy(scr, hand.index(cid), e.x, e.y)
                delay = now - e.tick / 20
                ok = False
                t1 = time.time()
                while time.time() - t1 < 1.5:
                    n, f2 = live.latest(newer_than=n, timeout=2)
                    if f2.players and f2.players[f2.own_index()].seq > seq0:
                        ok = True; break
                if not ok:
                    deselect()
                    print(f"  [{now:6.1f}s] {e.slug} 下牌失败，跳过")
                else:
                    print(f"  [{now:6.1f}s] {e.slug:12s} ({e.x},{e.y})  原时间 {e.tick / 20:6.1f}s  延迟 {delay:+.1f}s")
                log.append({"slug": e.slug, "x": e.x, "y": e.y, "orig_t": e.tick / 20, "t": round(now, 2), "ok": ok})
                queue.pop(0)
    finally:
        live.close()
    done = [x for x in log if x["ok"]]
    ontime = [x for x in done if x["t"] - x["orig_t"] <= 1.0]
    print(f"完成 {len(done)}/{len(evs)} 次出牌，其中准时（延迟 ≤1s）{len(ontime)} 次")
    (ROOT / "re" / "reenact_log.json").write_text(json.dumps(log, ensure_ascii=False, indent=1), encoding="utf-8")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)
