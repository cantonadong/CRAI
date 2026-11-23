"""生产用：把 HF 数据集中所有含野猪 2.6 一方（不限形态）的对局放进离线引擎重演，保存逐帧数据。

- 遍历 data/hf_il_replay/replays/*.parquet（下载还没完成也可以先跑，之后重跑会继续）。
- 断点续跑：data/resim_headless/hog_summary.jsonl 里已有的 tag 跳过。
- 输出：data/resim_headless/hog/<tag>.jsonl.gz（逐 tick 帧）和 <tag>.native.json.gz（实际喂给引擎的原生 JSON）。
- 游戏崩溃会由 headless_resim.ensure_game 自动重启；连续失败 5 次就暂停 60 秒。

用法：python -u scripts/hf_resim_hog.py [最多局数]
"""
import gzip
import json
import shutil
import sys
import time
from pathlib import Path

import pyarrow.parquet as pq

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import royaleapi_to_native as C  # noqa: E402
from batch_headless import load_frames, resolve_and_run  # noqa: E402
from check_resim import count_unexecuted  # noqa: E402

SRC = ROOT / "data/hf_il_replay/replays"
OUT = ROOT / "data/resim_headless/hog"
SUMMARY = ROOT / "data/resim_headless/hog_summary.jsonl"
TMP = ROOT / "data/resim_headless/_tmp_native.json"


def done_tags() -> set[str]:
    if not SUMMARY.exists():
        return set()
    return {json.loads(line)["tag"] for line in SUMMARY.read_text(encoding="utf-8").splitlines() if line.strip()}


def is_hog_payload(p: dict) -> bool:
    for side in ("team", "opponent"):
        pl = p["battle"].get(side, {}).get("players", [])
        if len(pl) == 1 and {C._split_form(c["card_key"])[0] for c in pl[0]["deck"]} == set(C.HOG26):
            return True
    return False


def main(limit: str = "0"):
    OUT.mkdir(parents=True, exist_ok=True)
    limit_n = int(limit)
    done = done_tags()
    n_new = n_ok = n_exact = fails = 0
    t0 = time.time()
    for part in sorted(SRC.glob("part-*.parquet")):
        t = pq.read_table(part, columns=["replay_tag", "payload_json"])
        for tag, pj in zip(t.column("replay_tag").to_pylist(), t.column("payload_json").to_pylist()):
            if tag in done:
                continue
            p = json.loads(pj)
            if not is_hog_payload(p):
                continue
            if limit_n and n_new >= limit_n:
                print("达到数量上限", flush=True)
                return
            row = {"tag": tag, "part": part.name}
            try:
                spec = C.spec_from_hf(p)
                row.update(orig=spec["meta"]["orig_crowns"], hog_owner=spec["meta"]["hog_owner"],
                           forms={s: spec["sides"][s]["forms"] for s in (0, 1)},
                           towers=[spec["sides"][s]["tower"] for s in (0, 1)],
                           battle_type=p["battle"].get("battle_type"),
                           result=p["battle"].get("result"))
                nat, meta, s = resolve_and_run(spec, TMP)
                frames = load_frames(s["out"])
                resim = {0: s["crowns"][0], 1: s["crowns"][1]}
                o = row["orig"]
                row.update(resim=resim, ticks=s["ticks"], plays=meta["plays"], abilities=len(meta["abilities"]),
                           abilities_used=meta["abilities_used"], legal=meta["legal_prefix"],
                           unknown=meta["unknown_cards"], unexecuted=count_unexecuted(nat, frames),
                           exact=resim == o,
                           same_winner=(resim[0] > resim[1]) - (resim[0] < resim[1]) == (o[0] > o[1]) - (o[0] < o[1]))
                with open(s["out"], "rb") as fi, gzip.open(OUT / f"{tag}.jsonl.gz", "wb", compresslevel=6) as fo:
                    shutil.copyfileobj(fi, fo)
                with gzip.open(OUT / f"{tag}.native.json.gz", "wt", encoding="utf-8") as fo:
                    json.dump(nat, fo, ensure_ascii=False)
                Path(s["out"]).unlink(missing_ok=True)
                n_ok += 1
                n_exact += row["exact"]
                fails = 0
            except Exception as e:  # noqa: BLE001
                row["error"] = f"{type(e).__name__}: {e}"
                fails += 1
            n_new += 1
            done.add(tag)
            with open(SUMMARY, "a", encoding="utf-8") as fh:
                fh.write(json.dumps(row, ensure_ascii=False, default=str) + "\n")
            if n_new % 20 == 0:
                el = time.time() - t0
                print(f"{time.strftime('%H:%M:%S')} 新完成 {n_new}（成功 {n_ok}，皇冠一致 {n_exact}），"
                      f"平均 {el / n_new:.1f}s/局，当前 {part.name}", flush=True)
            if fails >= 5:
                print(f"连续失败 {fails} 次，最近错误：{row.get('error')}；暂停 60 秒", flush=True)
                time.sleep(60)
                fails = 0
    print(f"全部完成：新完成 {n_new}，成功 {n_ok}，皇冠一致 {n_exact}", flush=True)


if __name__ == "__main__":
    main(*sys.argv[1:])
