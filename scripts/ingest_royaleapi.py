"""RoyaleAPI 回放接入：油猴脚本下载的 replayNNNN.json (+ replayNNNN.row.html) → 原生 JSON → 离线引擎重演 → 与原局皇冠核对。

不同来源分开存放（用户要求 Top200 通用数据与野猪 2.6 回放分开）：
  replay_top200/  →  data/royaleapi/top200/{native,frames}/<tag>.*  +  summary.jsonl  +  deck_pool.json
  replay/         →  data/royaleapi/hog/...（手动下载的野猪 2.6 回放）

  $env:CRAI_INSTANCE="1"
  .venv\\Scripts\\python.exe -u scripts/ingest_royaleapi.py [来源目录 replay_top200] [集合名 top200] [并行 2] [watch 1]

- watch=1：处理完后每 2 分钟检查一次清单，新下载的自动接上（边下载边接入）。断点续跑（按 summary 里的 tag 跳过）。
- 卡组/形态/等级/塔兵来自对战行 HTML（row.html）；没有 row 时退回"由出牌推断卡组 + 野猪方默认 2.6 形态"。
- 走 TCP 交互环境（与离线训练共用实例 1，每连接一局，互不干扰）。
- 英雄技能指令的 cgid 按 batch_headless.resolve_and_run 的办法逐个补全（每补一个重演一次）。
"""
import gzip
import json
import re
import sys
import threading
import time
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import royaleapi_to_native as C  # noqa: E402
from batch_headless import unit_cards_for  # noqa: E402
from crai.env.match import crowns_of  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402

MAX_TICKS = 6600
HOG26 = set(C.HOG26)


# ---------------------------------------------------------------------------
# 对战行 HTML：两个卡组网格（id="deck_..."），每张卡 <img data-card-key="hog-rider-ev1">，附近有等级文字
# ---------------------------------------------------------------------------
def parse_row(html: str) -> dict | None:
    starts = [m.start() for m in re.finditer(r'<div\b[^>]*\bid="deck_[^"]*"', html)]
    if len(starts) < 2:
        return None
    grids = []
    for i, s in enumerate(starts[:2]):
        seg = html[s: starts[i + 1] if i + 1 < len(starts) else len(html)]
        cards_, levels = [], {}
        imgs = list(re.finditer(r'<img\b[^>]*\bdata-card-key="([^"]+)"', seg))
        for j, m in enumerate(imgs):
            key = m.group(1).strip().lower()
            slug, el = C._split_form(key)
            if slug in [c[0] for c in cards_]:
                continue
            cards_.append((slug, el))
            after = seg[m.end(): imgs[j + 1].start() if j + 1 < len(imgs) else m.end() + 800]
            lv = re.search(r'(?:Lvl|Level|lvl)\.?\s*(\d{1,2})\b', after)
            if lv and 1 <= int(lv.group(1)) <= 16:
                levels[slug] = int(lv.group(1))
        grids.append({"cards": cards_[:8], "levels": levels})
    tower = 159000000
    for key in re.findall(r'data-card-key="([^"]+)"', html):
        if key.lower() in C.TOWER_TROOP_IDS:
            tower = C.TOWER_TROOP_IDS[key.lower()]
            break
    if any(len(g["cards"]) != 8 for g in grids):
        return None
    return {"grids": grids, "tower": tower}


def make_spec(json_path: Path, row_path: Path | None) -> tuple[dict, bool]:
    spec = C.spec_from_royaleapi(json_path, forms={})        # 先不给形态，下面用 row 覆盖
    row = parse_row(row_path.read_text(encoding="utf-8")) if row_path and row_path.exists() else None
    side_of = spec["meta"]["side_of"]
    if row is not None:
        played = {s: {p["card"] for p in spec["plays"] if p["side"] == s} for s in (0, 1)}
        g = row["grids"]
        decks = [{c for c, _ in x["cards"]} for x in g]
        # 第一个网格一般是本页玩家（team）；按"出过的牌必须在卡组里"核对，不符就交换
        team, opp = side_of["team"], side_of["opponent"]
        order = (0, 1)
        if not (played[team] <= decks[0] and played[opp] <= decks[1]) and (played[team] <= decks[1] and played[opp] <= decks[0]):
            order = (1, 0)
        for side, gi in ((team, order[0]), (opp, order[1])):
            x = g[gi]
            S = spec["sides"][side]
            S["deck"] = [c for c, _ in x["cards"] if c in C.cards.CARDS]
            S["forms"] = {c: el for c, el in x["cards"] if el and c in C.cards.CARDS}
            S["levels"] = dict(x["levels"])
            S["tower"] = row["tower"]          # 行里只稳定拿到一个塔兵；两边不同时以后再细分
    else:
        hog = spec["meta"].get("hog_owner")
        if hog is not None:
            spec["sides"][hog]["forms"] = dict(C.HOG26_FORMS)
    hog = [s for s in (0, 1) if set(spec["sides"][s]["deck"]) == HOG26]
    spec["meta"]["hog_owner"] = hog[0] if hog else None
    return spec, row is not None


# ---------------------------------------------------------------------------
# TCP 重演 + 技能 cgid 补全
# ---------------------------------------------------------------------------
def run_tcp(env: NativeEnv, nat: dict) -> list[dict]:
    body = json.dumps(nat, ensure_ascii=False).encode("utf-8")
    frame = env._request(f"RESET {len(body)}", body)
    if "error" in frame:
        raise RuntimeError(frame["error"])
    frames = [frame]
    while not frame.get("ended") and frame["tick"] < MAX_TICKS:
        frame = env.step(1)
        frames.append(frame)
    env._request("CLOSE")
    return frames


def resolve_and_run(env: NativeEnv, spec: dict):
    gids: dict[int, int | None] = {}
    n_ab = len(spec["abilities"])
    while True:
        nat, meta = C.build_from_spec(spec, gids)
        frames = run_tcp(env, nat)
        pending = [i for i in range(n_ab) if i not in gids]
        if not pending:
            return nat, meta, frames
        by_tick = {f["tick"]: f for f in frames}
        rerun = False
        for i in pending:
            a = meta["abilities"][i]
            f = by_tick.get(a["t2"] - 1)
            if f is None:
                gids[i] = None
                continue
            want = unit_cards_for(a["candidates"])
            alive = [e for e in f["entities"] if e["side"] == a["side"] and e["hp"] > 0]
            cands = [e for e in alive if e["card"] in want] or [e for e in alive if C.is_ability_unit(e["card"])]
            gids[i] = max(cands, key=lambda e: e["gid"])["gid"] if cands else None
            if gids[i] is not None:
                rerun = True
                break
        if not rerun:
            nat, meta = C.build_from_spec(spec, gids)
            return nat, meta, run_tcp(env, nat)


# ---------------------------------------------------------------------------
def write_pool(out: Path, rows: list[dict]):
    """精确重演的对局 → 双方卡组（带形态、塔兵）计数，给离线 PPO 当对手卡组池。"""
    cnt, info = Counter(), {}
    for r in rows:
        if not r.get("exact") or not r.get("row"):
            continue
        for s in ("0", "1"):
            d = r["decks"][s]
            if len(d) != 8:
                continue
            key = tuple(sorted(d))
            cnt[key] += 1
            info[key] = {"deck": sorted(d), "forms": r["forms"][s], "tower": r["tower"]}
    pool = [dict(info[k], count=n) for k, n in cnt.most_common()]
    (out / "deck_pool.json").write_text(json.dumps(pool, ensure_ascii=False, indent=1), encoding="utf-8")
    return len(pool)


def main(src_dir: str = "replay_top200", set_name: str = "top200", par: str = "2", watch: str = "1"):
    sys.stdout.reconfigure(line_buffering=True)
    src = ROOT / src_dir
    out = ROOT / "data/royaleapi" / set_name
    (out / "native").mkdir(parents=True, exist_ok=True)
    (out / "frames").mkdir(parents=True, exist_ok=True)
    summary = out / "summary.jsonl"
    lock = threading.Lock()

    def done_tags():
        if not summary.exists():
            return set()
        return {json.loads(x)["tag"] for x in summary.read_text(encoding="utf-8").splitlines() if x.strip()}

    def process(env, tag, info):
        jp = src / info["file"]
        rp = src / info["row"] if info.get("row") else src / info["file"].replace(".json", ".row.html")
        row = {"tag": tag, "file": info["file"], "set": set_name, "page": info.get("page")}
        try:
            spec, has_row = make_spec(jp, rp)
            side_of = spec["meta"]["side_of"]
            orig = {str(side_of["team"]): int(info["team_crowns"]), str(side_of["opponent"]): int(info["opponent_crowns"])}
            nat, meta, frames = resolve_and_run(env, spec)
            res = {str(k): v for k, v in crowns_of(frames[-1]).items()}
            with gzip.open(out / "native" / f"{tag}.native.json.gz", "wt", encoding="utf-8") as f:
                json.dump(nat, f, ensure_ascii=False)
            exact = res == orig
            if exact:                       # 只有和原局一致的才留逐 tick 帧（学习用）
                with gzip.open(out / "frames" / f"{tag}.jsonl.gz", "wt", encoding="utf-8") as f:
                    for fr in frames:
                        f.write(json.dumps(fr, separators=(",", ":")) + "\n")
            row.update(row=has_row, orig=orig, resim=res, exact=exact, ticks=frames[-1]["tick"],
                       hog_owner=spec["meta"]["hog_owner"], plays=meta["plays"], abilities=len(meta["abilities"]),
                       abilities_used=meta["abilities_used"], unknown=meta["unknown_cards"],
                       decks={str(s): spec["sides"][s]["deck"] for s in (0, 1)},
                       forms={str(s): spec["sides"][s]["forms"] for s in (0, 1)},
                       levels_found={str(s): len(spec["sides"][s]["levels"]) for s in (0, 1)},
                       tower=spec["sides"][0]["tower"])
        except Exception as e:  # noqa: BLE001
            row["error"] = f"{type(e).__name__}: {e}"
        return row

    while True:
        man_p = src / "_manifest.json"
        man = json.loads(man_p.read_text(encoding="utf-8")) if man_p.exists() else {}
        done = done_tags()
        todo = [(t, i) for t, i in sorted(man.items(), key=lambda kv: kv[1]["file"]) if t not in done]
        if todo:
            print(f"{time.strftime('%H:%M:%S')} 待接入 {len(todo)} 局（已完成 {len(done)}）")
            queue = list(todo)
            stats = Counter()

            def worker(_):
                env = NativeEnv(wait=300)
                while True:
                    with lock:
                        if not queue:
                            break
                        tag, info = queue.pop(0)
                    r = process(env, tag, info)
                    if "error" in r and ("Connection" in r["error"] or "OSError" in r["error"]):
                        try:
                            env.close()
                        except Exception:  # noqa: BLE001
                            pass
                        time.sleep(5)
                        env = NativeEnv(wait=300)
                    with lock:
                        with summary.open("a", encoding="utf-8") as f:
                            f.write(json.dumps(r, ensure_ascii=False) + "\n")
                        stats["n"] += 1
                        stats["exact"] += bool(r.get("exact"))
                        stats["hog"] += r.get("hog_owner") is not None
                        stats["err"] += "error" in r
                        if stats["n"] % 20 == 0 or not queue:
                            print(f"  {stats['n']}/{len(todo)} 与原局一致 {stats['exact']}  野猪2.6局 {stats['hog']}  出错 {stats['err']}")
                env.close()

            with ThreadPoolExecutor(int(par)) as ex:
                list(ex.map(worker, range(int(par))))
            rows = [json.loads(x) for x in summary.read_text(encoding="utf-8").splitlines() if x.strip()]
            print(f"  卡组池 {write_pool(out, rows)} 套 → {out / 'deck_pool.json'}")
        if watch != "1":
            break
        time.sleep(120)


if __name__ == "__main__":
    main(*sys.argv[1:])
