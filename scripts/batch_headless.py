"""批量：RoyaleAPI 回放 → 原生 JSON → 离线引擎重演 → 与原局皇冠对比。

用法：python -u scripts/batch_headless.py [回放目录 replay/]
结果追加到 data/resim_headless/_summary.jsonl，最后打印一致率。
"""
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import headless_resim as H  # noqa: E402
import royaleapi_to_native as C  # noqa: E402


def unexecuted(native: Path, frames: Path) -> int:
    out = subprocess.run([sys.executable, str(ROOT / "scripts/check_resim.py"), str(native), str(frames)],
                         capture_output=True, text=True, encoding="utf-8").stdout
    last = out.strip().splitlines()[-1]  # "指令 N 条，未执行 M 条"
    return int(last.split("未执行")[1].split("条")[0])


def load_frames(path) -> dict:
    out = {}
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        f = json.loads(line)
        out[f["tick"]] = f
    return out


def unit_cards_for(slugs) -> set[int]:
    """技能候选卡 → 场上单位的卡牌编号：英雄形态 = 203000000 + 编号后 6 位；冠军 = 卡牌 ID 本身。"""
    out = set()
    for s in slugs:
        try:
            cid = C.cards.card_id(s)
        except Exception:  # noqa: BLE001
            continue
        out.add(cid if cid in C.CHAMPION_IDS else 203000000 + cid % 1000000)
    return out


def resolve_and_run(spec: dict, dst: Path, quiet: bool = True):
    """逐个补全技能指令的 cgid：每轮重演后，在下一个技能时刻（t2-1）找该方场上有技能的单位
    （有候选卡时只看候选卡对应的单位，否则看全部英雄 203xxxxxx 和冠军），取对象 ID 最大（最新）的那个；
    找不到就放弃这一个。引擎是确定性的，补上一个技能不会改变它之前的过程。"""
    gids: dict[int, int | None] = {}
    n_ab = len(spec["abilities"])
    while True:
        nat, meta = C.build_from_spec(spec, gids)
        dst.write_text(json.dumps(nat, ensure_ascii=False), encoding="utf-8")
        s = H.main(str(dst), quiet=quiet)
        pending = [i for i in range(n_ab) if i not in gids]
        if not pending:
            return nat, meta, s
        frames = load_frames(s["out"])
        rerun = False
        for i in pending:
            a = meta["abilities"][i]
            f = frames.get(a["t2"] - 1)
            if f is None:              # 重演在这之前就结束了
                gids[i] = None
                continue
            # 英雄单位编号不一定是 203000000 + 卡牌编号后 6 位（英雄狂战士 26000102 → 203000076），
            # 所以先按候选找，找不到再退回该方任意英雄/冠军单位
            want = unit_cards_for(a["candidates"])
            alive = [e for e in f["entities"] if e["side"] == a["side"] and e["hp"] > 0]
            cands = [e for e in alive if e["card"] in want] or [e for e in alive if C.is_ability_unit(e["card"])]
            gids[i] = max(cands, key=lambda e: e["gid"])["gid"] if cands else None
            if gids[i] is not None:
                rerun = True
                break                  # 补上一个后重演，再看下一个
        if not rerun:
            nat, meta = C.build_from_spec(spec, gids)
            dst.write_text(json.dumps(nat, ensure_ascii=False), encoding="utf-8")
            return nat, meta, H.main(str(dst), quiet=quiet)


def main(src_dir: str = "replay"):
    src = ROOT / src_dir
    man = json.loads((src / "_manifest.json").read_text(encoding="utf-8"))
    summary_p = ROOT / "data/resim_headless/_summary.jsonl"
    summary_p.parent.mkdir(parents=True, exist_ok=True)
    rows = []
    for tag, info in sorted(man.items(), key=lambda kv: kv[1]["file"]):
        f = src / info["file"]
        row = {"file": info["file"], "orig": {"team": int(info["team_crowns"]), "opponent": int(info["opponent_crowns"])}}
        try:
            p = ROOT / "data/native_replays" / info["file"]
            p.parent.mkdir(parents=True, exist_ok=True)
            nat, meta, s = resolve_and_run(C.spec_from_royaleapi(f), p)
            resim = {rs: s["crowns"][side] for rs, side in meta["side_of"].items()}
            o = row["orig"]
            row.update(resim=resim, plays=meta["plays"], abilities=len(meta["abilities"]),
                       abilities_used=meta["abilities_used"],
                       unknown=meta["unknown_cards"], unexecuted=unexecuted(p, Path(s["out"])),
                       exact=resim == o,
                       same_winner=(resim["team"] > resim["opponent"]) == (o["team"] > o["opponent"])
                       and (resim["team"] == resim["opponent"]) == (o["team"] == o["opponent"]))
        except Exception as e:  # noqa: BLE001
            row["error"] = f"{type(e).__name__}: {e}"
        print(json.dumps(row, ensure_ascii=False), flush=True)
        rows.append(row)
        with open(summary_p, "a", encoding="utf-8") as fh:
            fh.write(json.dumps(row, ensure_ascii=False) + "\n")
    ok = [r for r in rows if "error" not in r]
    def rate(rs, k):
        return f"{sum(r[k] for r in rs)}/{len(rs)}"
    no_ab = [r for r in ok if r["abilities"] == 0]
    print(f"\n完成 {len(ok)}/{len(rows)}；皇冠完全一致 {rate(ok, 'exact')}，胜负一致 {rate(ok, 'same_winner')}")
    print(f"无英雄技能的局：皇冠完全一致 {rate(no_ab, 'exact')}，胜负一致 {rate(no_ab, 'same_winner')}")


if __name__ == "__main__":
    main(*sys.argv[1:])
