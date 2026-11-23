"""把 RoyaleAPI 回放注入游戏内回放，让游戏引擎完整重演（仅本机回放实验）。

流程：
  1. 打开对战日志第 1 局回放（作底），开播后立即读取：双方卡组元素/手牌/队列地址、全部出牌指令对象
  2. 由 RoyaleAPI 回放构建双方 8 张卡组（未出过的卡用填充卡）、反推合法的开局手牌/队列
  3. 生成写入清单：卡组元素卡牌指针/等级(16级)/关闭进化；开局手牌/队列；按 tick 改写全部指令；多余指令推迟到对局结束后
  4. poke_batch 一次写入；之后录制重演过程到 data/resim/，结束时对比皇冠数
用法: python -u scripts/inject_replay.py <royaleapi回放文件>
"""
import itertools
import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.data import cards  # noqa: E402
from crai.data.dataset import HOG26, side_at_bottom  # noqa: E402
from crai.data.replay import load_replay  # noqa: E402
from crai.device import ADB, SERIAL, as_root  # noqa: E402
from ui import go_home, tap  # noqa: E402

FILLERS = ["knight", "archers", "goblins", "spear-goblins", "bats", "zap", "arrows", "bomber", "valkyrie", "giant"]
META_FIXED = 0x3C00


def sh(cmd):
    return subprocess.run([ADB, "-s", SERIAL, "shell", cmd], capture_output=True, text=True).stdout


def card_ptrs(pid):
    out = sh(as_root(f"/data/local/tmp/card_table {pid}"))
    return {int(a): int(b, 16) for a, b in (l.split() for l in out.splitlines() if len(l.split()) == 2)}


def legal_order(deck8, plays):
    """在 C(8,4)*4! 种 (开局手牌, 队列顺序) 中找一种能让出牌序列全部合法的；返回 (hand4, queue4) 或 None。"""
    for hand in itertools.combinations(deck8, 4):
        rest = [c for c in deck8 if c not in hand]
        for queue in itertools.permutations(rest):
            h, q, ok = set(hand), list(queue), True
            for c in plays:
                if c not in h:
                    ok = False; break
                h.remove(c); q.append(c); h.add(q.pop(0))
            if ok:
                return list(hand), list(queue)
    return None


def parse_decks(txt):
    players = []
    for line in txt.splitlines():
        if line.startswith("player["):
            kv = dict(p.split("=", 1) for p in line.split() if "=" in p)
            players.append({"side": int(kv["side"]), "hand_ptr": int(kv["hand_ptr"], 16),
                            "queue_ptr": int(kv["queue_ptr"], 16), "elems": [], "card_ptrs": []})
        elif line.strip().startswith("deck["):
            kv = dict(p.split("=", 1) for p in line.split() if "=" in p)
            players[-1]["elems"].append(int(kv["elem"], 16))
            players[-1]["card_ptrs"].append(int(kv["card"], 16))
    return players


def main(path):
    sys.stdout.reconfigure(line_buffering=True)
    r = load_replay(path)
    # ---------- 1. 打开底子回放并读取结构 ----------
    assert go_home()
    tap(983, 203); time.sleep(3); tap(647, 130); time.sleep(4); tap(900, 917)
    t_open = time.time()
    time.sleep(1.5)
    pid = sh("pidof nullsroyale.rel.free").strip()
    from crai.reader.stream import snapshot
    f = snapshot()
    hp = parse_decks(sh(as_root(f"/data/local/tmp/dump_decks {pid} {f.roster}")))
    cmds = sorted((l.split() for l in sh(as_root(f"/data/local/tmp/list_cmds {pid}")).splitlines() if len(l.split()) == 7),
                  key=lambda x: int(x[1]))
    ptr = card_ptrs(pid)
    print(f"开播 {time.time() - t_open:.1f}s：底子指令 {len(cmds)} 条，首条 tick {cmds[0][1] if cmds else '-'}，卡牌指针 {len(ptr)}")
    # 底子玩家标识 -> 底子 player 下标（按指令卡牌属于哪方卡组）
    pid_of = {}
    for c in cmds:
        cp = None
        for k, p in enumerate(hp):
            card_id = int(c[6])
            if any(ptr.get(card_id) == x for x in p["card_ptrs"]):
                cp = k
        if cp is not None:
            pid_of[cp] = int(c[3])
    # ---------- 2. RoyaleAPI 双方 -> 底子双方：野猪 2.6 一方固定放在下方（底子 side=1） ----------
    hog_side = next((s for s in ("team", "opponent")
                     if set((r.team if s == "team" else r.opponent).card_counts) == set(HOG26)), None)
    if hog_side is None:
        hog_side = "team" if side_at_bottom(r, "team") else "opponent"
    flip = not side_at_bottom(r, hog_side)       # 原局野猪 2.6 在上方 -> 全部坐标旋转 180°
    other = "opponent" if hog_side == "team" else "team"
    writes, plan = [], {}
    for k, p in enumerate(hp):
        rside = hog_side if p["side"] == 1 else other
        seq = [x.card for x in r.side_plays(rside, include_abilities=False) if cards.card_id(x.card) in ptr]
        deck = list(dict.fromkeys(seq))[:8]
        for fl in FILLERS:
            if len(deck) >= 8:
                break
            if fl not in deck:
                deck.append(fl)
        order = legal_order(deck, seq)
        if order is None:
            sys.exit(f"{rside} 找不到合法的开局顺序")
        hand, queue = order
        slots = hand + queue                      # deck index 0..3 = 开局手牌，4..7 = 队列顺序
        plan[k] = {"rside": rside, "slots": slots, "player_id": pid_of.get(k)}
        name = r.team.name if rside == "team" else r.opponent.name
        print(f"底子 player[{k}] side={p['side']} <- {rside}（{name}）卡组 {slots}")
        for d, slug in enumerate(slots):
            e = p["elems"][d]
            writes += [(e + 0x10, 8, ptr[cards.card_id(slug)]), (e + 0x18, 4, cards.level16_index(slug)),
                       (e + 0x1C, 4, 0), (e + 0x24, 4, 0)]
        for i in range(4):
            writes += [(p["hand_ptr"] + 4 * i, 4, i), (p["queue_ptr"] + 4 * i, 4, 4 + i)]
    # ---------- 3. 指令 ----------
    side_to_k = {plan[k]["rside"]: k for k in plan}
    plays = sorted((x for x in r.plays if not x.is_ability and cards.card_id(x.card) in ptr), key=lambda x: x.tick)
    if len(plays) > len(cmds):
        print(f"警告：RoyaleAPI 出牌 {len(plays)} 条多于底子指令 {len(cmds)} 条，截断")
    for i, c in enumerate(cmds):
        a = int(c[0], 16)
        if i < len(plays):
            x = plays[i]
            k = side_to_k[x.side]
            d = plan[k]["slots"].index(x.card)
            meta = (cards.cost(x.card) << 28) | ((d + 1) << 22) | META_FIXED
            px, py = (18000 - x.x, 32000 - x.y) if flip else (x.x, x.y)
            writes += [(a + 0x0C, 4, x.tick), (a + 0x10, 4, x.tick - 20), (a + 0x18, 4, plan[k]["player_id"]),
                       (a + 0x28, 4, px), (a + 0x2C, 4, py), (a + 0x38, 8, ptr[cards.card_id(x.card)]),
                       (a + 0x48, 4, meta - (1 << 32) if meta >= 1 << 31 else meta)]
        else:
            writes += [(a + 0x0C, 4, 99000 + i), (a + 0x10, 4, 98980 + i)]
    # ---------- 4. 写入 ----------
    lst = ROOT / "re" / "inject_writes.txt"
    lst.write_text("\n".join(f"{a:x} {s} {v}" for a, s, v in writes) + "\n", encoding="ascii")
    subprocess.run([ADB, "-s", SERIAL, "push", str(lst), "/data/local/tmp/inject_writes.txt"], capture_output=True)
    res = sh(as_root(f"/data/local/tmp/poke_batch {pid} /data/local/tmp/inject_writes.txt"))
    print(f"开播 {time.time() - t_open:.1f}s 写入完成：{res.strip()}（RoyaleAPI 出牌 {len(plays)} 条，坐标翻转={flip}）")
    (ROOT / "re" / "inject_plan.json").write_text(json.dumps(plan, ensure_ascii=False, indent=1), encoding="utf-8")
    return record_resim(path, r, hog_side, len(plays) <= len(cmds))


def record_resim(path, r, hog_side, complete):
    """录制重演全过程到 data/resim/<回放名>.jsonl，结束后核对结局（野猪 2.6 一方在下方 = side 1）。"""
    from dataclasses import asdict
    from crai.reader.stream import LiveReader
    out_dir = ROOT / "data" / "resim"; out_dir.mkdir(parents=True, exist_ok=True)
    out = out_dir / (Path(path).stem + ".jsonl")
    live = LiveReader(period_ms=33, rescan=1)
    n, last, last_save, frames, t0 = -1, None, 0.0, 0, time.time()
    try:
        with out.open("w", encoding="utf-8") as fh:
            while time.time() - t0 < 420:
                n, f = live.latest(newer_than=n, timeout=10)
                if not f.battle:
                    if last is not None:
                        break
                    continue
                if last is None or time.time() - last_save >= 0.1 or \
                        tuple(p.seq for p in f.players) != tuple(p.seq for p in last.players):
                    fh.write(json.dumps({"wall": round(time.time(), 3), **asdict(f)}, ensure_ascii=False) + "\n")
                    frames += 1; last_save = time.time()
                last = f
    finally:
        live.close()
    # 结局：各方剩余塔（hp>0）
    towers = {s: sum(1 for e in last.entities if e.card == -1 and e.side == s and e.hp > 0) for s in (0, 1)} if last else {}
    crowns_resim = {"hog": 3 - towers.get(0, 3), "other": 3 - towers.get(1, 3)}  # 野猪 2.6 在 side 1，拆的是 side 0 的塔
    man = json.loads((Path(path).parent / "_manifest.json").read_text(encoding="utf-8"))
    info = next((v for v in man.values() if v["file"] == Path(path).name), None)
    orig = None
    if info:
        tc, oc = int(info["team_crowns"]), int(info["opponent_crowns"])
        orig = {"hog": tc if hog_side == "team" else oc, "other": oc if hog_side == "team" else tc}
    print(f"重演录制 {frames} 帧 -> {out.name}；重演皇冠 {crowns_resim}，原局皇冠 {orig}，指令完整={complete}")
    return {"file": Path(path).name, "frames": frames, "resim": crowns_resim, "orig": orig, "complete": complete}


if __name__ == "__main__":
    main(sys.argv[1])
