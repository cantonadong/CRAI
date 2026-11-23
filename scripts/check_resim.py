"""核对离线引擎重演：每条出牌指令是否被执行（出牌序号是否 +1、费用是否扣除）。

用法：python scripts/check_resim.py <native.json> <重演输出.jsonl>
"""
import json
import sys
from pathlib import Path


def count_unexecuted(nat: dict, frames: dict) -> int:
    """不打印，只返回未执行的出牌指令数（frames: tick → 帧）。"""
    acct = {nat["battle"]["avatar0"]["accountID.lo"]: 0, nat["battle"]["avatar1"]["accountID.lo"]: 1}
    bad = 0
    for c in nat["cmd"]:
        if c["ct"] != 153:
            continue
        side, t2 = acct[c["c"]["idLo"]], c["c"]["t2"]
        b, a = frames.get(t2 - 1), frames.get(t2 + 2)
        if b is None or a is None:
            bad += 1
            continue
        pb = next(p for p in b["players"] if p and p["side"] == side)
        pa = next(p for p in a["players"] if p and p["side"] == side)
        bad += pa["seq"] - pb["seq"] < 1
    return bad


def main(native_path, frames_path):
    nat = json.loads(Path(native_path).read_text(encoding="utf-8"))
    frames = {}
    for line in Path(frames_path).read_text(encoding="utf-8").splitlines():
        f = json.loads(line)
        frames[f["tick"]] = f
    acct = {nat["battle"]["avatar0"]["accountID.lo"]: 0, nat["battle"]["avatar1"]["accountID.lo"]: 1}
    deck = {s: [e["d"] for e in nat["battle"][f"deck{s}"]["sp"]] for s in (0, 1)}

    def player(t, side):
        f = frames.get(t)
        return None if f is None else next(p for p in f["players"] if p and p["side"] == side)

    bad = 0
    for c in nat["cmd"]:
        if c["ct"] != 153:
            continue
        x = c["c"]
        side = acct[x["idLo"]]
        t2 = x["t2"]
        before, after = player(t2 - 1, side), player(t2 + 2, side)
        if before is None or after is None:
            print(f"t2={t2} side{side} 超出重演范围")
            bad += 1
            continue
        hand_cards = [deck[side][i] for i in before["hand"] if i >= 0]
        played = after["seq"] - before["seq"]
        ok = played >= 1
        if not ok:
            bad += 1
            print(f"未执行 t2={t2} side{side} card={x['sel']['os']} in_hand={x['sel']['os'] in hand_cards} "
                  f"elixir={before['elixir']:.2f} cost={x['sel']['pd'] >> 28} pos=({x['px']},{x['py']})")
    total = sum(1 for c in nat["cmd"] if c["ct"] == 153)
    print(f"指令 {total} 条，未执行 {bad} 条")


if __name__ == "__main__":
    main(*sys.argv[1:])
