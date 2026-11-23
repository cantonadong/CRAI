"""批量重演 FirstLight HF 数据集（VanguardX101/IL_Replay）里的回放，核对结局。

用法：python -u scripts/hf_batch.py <part.parquet> [数量 50] [all|hog]
  hog：只要有一方是野猪 2.6（不论形态）的对局。
结果追加到 data/resim_headless/hf_summary.jsonl；逐帧数据在 data/resim_headless/hf/<tag>.jsonl。
"""
import json
import sys
import time
from pathlib import Path

import pyarrow.parquet as pq

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import royaleapi_to_native as C  # noqa: E402
from batch_headless import resolve_and_run, unexecuted  # noqa: E402

OUT = ROOT / "data/resim_headless/hf"
NAT = ROOT / "data/native_replays/hf"


def main(part: str, limit: str = "50", mode: str = "all"):
    OUT.mkdir(parents=True, exist_ok=True)
    NAT.mkdir(parents=True, exist_ok=True)
    t = pq.read_table(part, columns=["replay_tag", "payload_json", "team_crowns", "opponent_crowns"])
    summary = ROOT / "data/resim_headless/hf_summary.jsonl"
    rows, n = [], int(limit)
    t0 = time.time()
    for tag, pj in zip(t.column("replay_tag").to_pylist(), t.column("payload_json").to_pylist()):
        if len(rows) >= n:
            break
        payload = json.loads(pj)
        try:
            spec = C.spec_from_hf(payload)
        except Exception as e:  # noqa: BLE001
            continue
        if mode == "hog" and spec["meta"]["hog_owner"] is None:
            continue
        row = {"tag": tag, "orig": spec["meta"]["orig_crowns"], "hog_owner": spec["meta"]["hog_owner"],
               "forms": {s: spec["sides"][s]["forms"] for s in (0, 1)},
               "towers": [spec["sides"][s]["tower"] for s in (0, 1)]}
        try:
            dst = NAT / f"{tag}.json"
            nat, meta, s = resolve_and_run(spec, dst)
            frames_p = OUT / f"{tag}.jsonl"
            Path(s["out"]).replace(frames_p)
            resim = {0: s["crowns"][0], 1: s["crowns"][1]}
            o = row["orig"]
            row.update(resim=resim, plays=meta["plays"], abilities=len(meta["abilities"]),
                       abilities_used=meta["abilities_used"], legal=meta["legal_prefix"],
                       unknown=meta["unknown_cards"], unexecuted=unexecuted(dst, frames_p),
                       exact=resim == o, same_winner=(resim[0] > resim[1]) - (resim[0] < resim[1]) == (o[0] > o[1]) - (o[0] < o[1]))
        except Exception as e:  # noqa: BLE001
            row["error"] = f"{type(e).__name__}: {e}"
        rows.append(row)
        print(json.dumps(row, ensure_ascii=False, default=str), flush=True)
        with open(summary, "a", encoding="utf-8") as fh:
            fh.write(json.dumps(row, ensure_ascii=False, default=str) + "\n")
    ok = [r for r in rows if "error" not in r]
    print(f"\n{len(ok)}/{len(rows)} 完成，用时 {time.time() - t0:.0f}s；"
          f"皇冠完全一致 {sum(r['exact'] for r in ok)}/{len(ok)}，胜负一致 {sum(r['same_winner'] for r in ok)}/{len(ok)}")


if __name__ == "__main__":
    main(*sys.argv[1:])
