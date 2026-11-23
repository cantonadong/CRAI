"""模仿学习数据集 v2：用离线引擎重演的逐帧状态（单位、血量、双方圣水和手牌）。

数据来源：scripts/hf_resim_hog.py 的输出
  data/resim_headless/hog/<tag>.jsonl.gz        逐 tick 帧（players: side/elixir/seq/hand/next/deck；entities）
  data/resim_headless/hog/<tag>.native.json.gz  喂给引擎的原生 JSON（cmd：出牌和技能指令，带下达 tick t）
  data/resim_headless/hog_summary.jsonl         hog_owner（野猪 2.6 一方的 side）、原局/重演皇冠

视角：野猪方，统一旋转到"己方在下方"（y 大的一侧；side 0 的对局做 180° 旋转）。
决策点：每 STEP 个 tick 一个。窗口 [t, t+STEP) 内己方有指令（按下达 tick）时标签为该指令，否则为"不出"。
  label = [类型(0 不出 / 1 出牌 / 2 技能), 手牌槽位 0-3（出牌时）, 格子 x 0-17, 格子 y 0-31]
特征（全部来自同一时刻的帧，实战中读取器都能读到）：
  scal  [S]     时间、阶段 one-hot(3)、双方圣水、双方 3 座塔血量比例、己方/对方出牌数
  cards [10]    己方手牌 4 + 下一张，对方手牌 4 + 下一张（词表下标）
  ents  [E, F]  单位：词表下标、是否己方、x、y、血量比例、log 最大血量、kind/15、有效
重演结局与原局不一致的对局，只保留到第一条未执行的指令之前（之后已偏离原局）。
"""

from __future__ import annotations

import gzip
import json
import math
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "data/resim_headless/hog"
SUMMARY = ROOT / "data/resim_headless/hog_summary.jsonl"
OUT = ROOT / "data/il_v2"
VOCAB_P = OUT / "vocab.json"

STEP = 5
E_MAX = 40
E_FEAT = 8
OT_TICK = 3600
DOUBLE_TICK = 2400
S_DIM = 1 + 3 + 2 + 6 + 2


class Vocab:
    """卡牌/单位编号 → 下标。0 = 空，1 = 未知。新编号追加并持久化，保证跨批次稳定。"""

    def __init__(self):
        self.ids: dict[int, int] = {}
        if VOCAB_P.exists():
            self.ids = {int(k): v for k, v in json.loads(VOCAB_P.read_text()).items()}
        self.dirty = False

    def __call__(self, cid: int) -> int:
        if cid is None or cid < 0:
            return 1
        i = self.ids.get(cid)
        if i is None:
            i = self.ids[cid] = len(self.ids) + 2
            self.dirty = True
        return i

    def save(self):
        if self.dirty:
            VOCAB_P.parent.mkdir(parents=True, exist_ok=True)
            VOCAB_P.write_text(json.dumps({str(k): v for k, v in sorted(self.ids.items(), key=lambda kv: kv[1])}))
            self.dirty = False


def _rot(x, y, flip):
    return (18000 - x, 32000 - y) if flip else (x, y)


def tower_hp(entities, own_side, flip):
    """[己方 左/右/国王, 对方 左/右/国王] 血量比例（视角坐标下 x<9000 为左）。"""
    out = np.zeros(6, np.float32)
    for e in entities:
        if e["card"] != -1 or e["max_hp"] <= 0:
            continue
        x, _ = _rot(e["x"], e["y"], flip)
        k = 2 if e["x"] == 9000 else (0 if x < 9000 else 1)
        base = 0 if e["side"] == own_side else 3
        out[base + k] = max(0.0, e["hp"]) / e["max_hp"]
    return out


def build_game(tag: str, own: int, vocab: Vocab, cut_tick: int | None = None):
    frames = {}
    with gzip.open(SRC / f"{tag}.jsonl.gz", "rt", encoding="utf-8") as f:
        for line in f:
            fr = json.loads(line)
            frames[fr["tick"]] = fr
    with gzip.open(SRC / f"{tag}.native.json.gz", "rt", encoding="utf-8") as f:
        nat = json.load(f)
    acct = {nat["battle"]["avatar0"]["accountID.lo"]: 0, nat["battle"]["avatar1"]["accountID.lo"]: 1}
    flip = own == 0          # 己方 side 0 在 y 小的一侧 → 旋转到下方
    end = max(frames)
    if cut_tick is not None:
        end = min(end, cut_tick)
    # 己方指令按决策窗口归位
    own_cmds = {}
    for c in nat["cmd"]:
        x = c["c"]
        if acct.get(x["idLo"]) != own:
            continue
        t = x["t"]
        if t < 0 or t > end:
            continue
        own_cmds.setdefault(t - t % STEP, c)
    rows_s, rows_c, rows_e, rows_l = [], [], [], []
    for t in range(0, end + 1, STEP):
        fr = frames.get(t)
        if fr is None:
            continue
        ps = {p["side"]: p for p in fr["players"] if p}
        if own not in ps or 1 - own not in ps:
            continue
        me, op = ps[own], ps[1 - own]

        def hand_ids(p):
            d = p["deck"]
            h = [d[i] if 0 <= i < 8 else -1 for i in p["hand"]]
            n = d[p["next"][0]] if 0 <= p["next"][0] < 8 else -1
            return h, n

        mh, mn = hand_ids(me)
        oh, on = hand_ids(op)
        cards = np.array([vocab(c) for c in mh + [mn] + oh + [on]], np.int16)
        phase = [t < DOUBLE_TICK, DOUBLE_TICK <= t < OT_TICK, t >= OT_TICK]
        scal = np.concatenate([[t / OT_TICK], np.array(phase, np.float32), [me["elixir"] / 10, op["elixir"] / 10],
                               tower_hp(fr["entities"], own, flip), [me["seq"] / 50, op["seq"] / 50]]).astype(np.float32)
        ents = np.zeros((E_MAX, E_FEAT), np.float32)
        units = [e for e in fr["entities"] if e["card"] != -1]
        units.sort(key=lambda e: (e["side"] != own, e["gid"]))
        for i, e in enumerate(units[:E_MAX]):
            x, y = _rot(e["x"], e["y"], flip)
            mx = max(e["max_hp"], 1)
            ents[i] = [vocab(e["card"]), float(e["side"] == own), x / 18000, y / 32000,
                       max(e["hp"], 0) / mx if e["max_hp"] > 0 else 1.0,
                       math.log1p(max(e["max_hp"], 0)) / 10, e["kind"] / 15, 1.0]
        label = [0, -1, -1, -1]
        c = own_cmds.get(t)
        if c is not None:
            if c["ct"] == 189:
                label = [2, -1, -1, -1]
            else:
                x = c["c"]
                cid = x["sel"]["os"]
                if cid in mh:
                    px, py = _rot(x["px"], x["py"], flip)
                    label = [1, mh.index(cid), min(17, max(0, px // 1000)), min(31, max(0, py // 1000))]
        rows_s.append(scal); rows_c.append(cards); rows_e.append(ents); rows_l.append(label)
    if not rows_s:
        return None
    return {"scal": np.stack(rows_s).astype(np.float16), "cards": np.stack(rows_c),
            "ents": np.stack(rows_e).astype(np.float16), "label": np.array(rows_l, np.int8)}


def build_all(only_new: bool = True):
    OUT.mkdir(parents=True, exist_ok=True)
    vocab = Vocab()
    rows = [json.loads(line) for line in SUMMARY.read_text(encoding="utf-8").splitlines() if line.strip()]
    n_new = n_samples = n_plays = 0
    for r in rows:
        if "error" in r or r.get("hog_owner") is None:
            continue
        dst = OUT / f"{r['tag']}.npz"
        if only_new and dst.exists():
            continue
        if not (SRC / f"{r['tag']}.jsonl.gz").exists():
            continue
        # 结局不一致：截到第一条未执行指令之前（之后已偏离原局）；结局一致：全部保留
        cut = None if r.get("exact") else _first_bad_tick(r["tag"])
        g = build_game(r["tag"], int(r["hog_owner"]), vocab, cut)
        if g is None:
            continue
        np.savez_compressed(dst, **g, won=np.int8(_won(r)), exact=np.int8(bool(r.get("exact"))))
        n_new += 1
        n_samples += len(g["label"])
        n_plays += int((g["label"][:, 0] > 0).sum())
        if n_new % 200 == 0:
            vocab.save()
    vocab.save()
    print(f"新增对局 {n_new}，决策点 {n_samples}，其中出牌/技能 {n_plays}；词表 {len(vocab.ids) + 2}")


def _won(r) -> int:
    o = r["orig"]
    own = str(r["hog_owner"])
    other = str(1 - int(own))
    return int(o[own] > o[other]) if own in o else int(o[int(own)] > o[int(other)])


def _first_bad_tick(tag: str) -> int | None:
    """重演中第一条未执行的出牌指令的下达 tick。"""
    with gzip.open(SRC / f"{tag}.native.json.gz", "rt", encoding="utf-8") as f:
        nat = json.load(f)
    acct = {nat["battle"]["avatar0"]["accountID.lo"]: 0, nat["battle"]["avatar1"]["accountID.lo"]: 1}
    frames = {}
    with gzip.open(SRC / f"{tag}.jsonl.gz", "rt", encoding="utf-8") as f:
        for line in f:
            fr = json.loads(line)
            frames[fr["tick"]] = fr
    for c in sorted(nat["cmd"], key=lambda c: c["c"]["t2"]):
        if c["ct"] != 153:
            continue
        side, t2 = acct[c["c"]["idLo"]], c["c"]["t2"]
        b, a = frames.get(t2 - 1), frames.get(t2 + 2)
        if b is None or a is None:
            return c["c"]["t"]
        pb = next(p for p in b["players"] if p and p["side"] == side)
        pa = next(p for p in a["players"] if p and p["side"] == side)
        if pa["seq"] - pb["seq"] < 1:
            return c["c"]["t"]
    return None


if __name__ == "__main__":
    build_all()
