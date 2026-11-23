"""用新代理库（带攻击目标 atk、技能控制器 abil）把已重演过的 HF 野猪局重新跑一遍，录逐 tick 帧。

输入：data/resim_headless/hog/<tag>.native.json.gz（第一轮重演实际喂给引擎的原生 JSON，技能已补全）
     data/resim_headless/hog_summary.jsonl（只取 exact=true 的局；可选只取用户形态）
输出：data/resim_headless/hog_v2/<tag>.jsonl.gz，汇总 data/resim_headless/hog_v2_summary.jsonl（断点续跑）

走 TCP 交互环境（RESET 整局 JSON → 逐 tick STEP），多个连接并行（代理库每连接一局）。

  $env:CRAI_INSTANCE="1"
  .venv\\Scripts\\python.exe -u scripts/resim_v2.py [并行 8] [只要用户形态 1] [最多局数 0]
"""
import gzip
import json
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.env.match import crowns_of  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402

SRC = ROOT / "data/resim_headless/hog"
SUMMARY_IN = ROOT / "data/resim_headless/hog_summary.jsonl"
OUT = ROOT / "data/resim_headless/hog_v2"
SUMMARY = ROOT / "data/resim_headless/hog_v2_summary.jsonl"
USER_FORMS = {"cannon": 1, "musketeer": 2, "skeletons": 1}
MAX_TICKS = 6600


def todo(user_only: bool) -> list[dict]:
    done = set()
    if SUMMARY.exists():
        done = {json.loads(x)["tag"] for x in SUMMARY.read_text(encoding="utf-8").splitlines() if x.strip()}
    out = []
    for line in SUMMARY_IN.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        r = json.loads(line)
        if not r.get("exact") or r["tag"] in done or not (SRC / f"{r['tag']}.native.json.gz").exists():
            continue
        if user_only and r["forms"].get(str(r["hog_owner"]), {}) != USER_FORMS:
            continue
        out.append(r)
    return out


def run_one(env: NativeEnv, r: dict) -> dict:
    body = gzip.decompress((SRC / f"{r['tag']}.native.json.gz").read_bytes())
    frame = env._request(f"RESET {len(body)}", body)
    if "error" in frame:
        raise RuntimeError(frame["error"])
    frames = [frame]
    while not frame.get("ended") and frame["tick"] < MAX_TICKS:
        frame = env.step(1)
        frames.append(frame)
    env._request("CLOSE")
    dst = OUT / f"{r['tag']}.jsonl.gz"
    with gzip.open(dst, "wt", encoding="utf-8") as f:
        for fr in frames:
            f.write(json.dumps(fr, separators=(",", ":")) + "\n")
    cr = crowns_of(frames[-1])
    has_atk = sum("atk" in e for e in frames[-1]["entities"])
    has_abil = any(p.get("abil") for p in frames[-1]["players"] if p)
    return {"tag": r["tag"], "hog_owner": r["hog_owner"], "ticks": frames[-1]["tick"], "resim": cr,
            "orig": r["orig"], "same_as_v1": {str(k): v for k, v in cr.items()} == {str(k): v for k, v in r["resim"].items()},
            "atk_last": has_atk, "abil": has_abil}


def main(par: str = "8", user_only: str = "1", limit: str = "0"):
    sys.stdout.reconfigure(line_buffering=True)
    OUT.mkdir(parents=True, exist_ok=True)
    items = todo(user_only == "1")
    if int(limit):
        items = items[:int(limit)]
    print(f"待重演 {len(items)} 局，并行 {par}")
    lock = threading.Lock()
    log = SUMMARY.open("a", encoding="utf-8")
    stats = {"ok": 0, "same": 0, "fail": 0}
    t0 = time.time()
    queue = list(items)

    def worker(_):
        env = NativeEnv(wait=300)
        while True:
            with lock:
                if not queue:
                    break
                r = queue.pop(0)
            try:
                s = run_one(env, r)
            except (ConnectionError, OSError, RuntimeError, ValueError) as e:
                with lock:
                    stats["fail"] += 1
                    print(f"  {r['tag']} 失败：{e}")
                try:
                    env.close()
                except Exception:  # noqa: BLE001
                    pass
                time.sleep(5)
                from headless_resim import ensure_game
                ensure_game()
                env = NativeEnv(wait=300)
                continue
            with lock:
                log.write(json.dumps(s, ensure_ascii=False) + "\n"); log.flush()
                stats["ok"] += 1
                stats["same"] += s["same_as_v1"]
                n = stats["ok"]
                if n % 25 == 0:
                    el = time.time() - t0
                    print(f"  {n}/{len(items)} 与第一轮结局相同 {stats['same']}  失败 {stats['fail']}  "
                          f"{el / n:.2f}s/局  预计还要 {(len(items) - n) * el / n / 60:.0f} 分钟")
        env.close()

    with ThreadPoolExecutor(int(par)) as ex:
        list(ex.map(worker, range(int(par))))
    log.close()
    print(json.dumps(stats, ensure_ascii=False))


if __name__ == "__main__":
    main(*sys.argv[1:])
