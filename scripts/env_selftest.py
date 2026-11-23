"""交互式环境自检：同一局，"开局时一次性载入全部指令"和"开局不带指令、在下达 tick 逐条注入"两种方式，
结局和每个 tick 的状态应该完全一致。

用法：python scripts/env_selftest.py [原生 JSON，缺省 re/native_replay/ladder_last.json]
"""
import copy
import json
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
from crai.env.native_env import NativeEnv  # noqa: E402


def sig(f):
    """帧的比较签名：tick、双方圣水和出牌数、全部单位的 (卡牌, 阵营, x, y, 血量)。"""
    ps = sorted((p["side"], round(p["elixir"], 3), p["seq"]) for p in f["players"] if p)
    es = sorted((e["card"], e["side"], e["x"], e["y"], e["hp"]) for e in f["entities"])
    return f["tick"], ps, es


def run(env, native, inject):
    start = copy.deepcopy(native)
    cmds = sorted(start["cmd"], key=lambda c: c["c"]["t"])
    if inject:
        start["cmd"] = []
    f = env.reset(start)
    frames = [sig(f)]
    i = 0
    t0 = time.time()
    while True:
        if inject:
            while i < len(cmds) and cmds[i]["c"]["t"] <= f["tick"]:
                assert env.inject(cmds[i]), f"注入失败 {cmds[i]}"
                i += 1
        f = env.step(1)
        frames.append(sig(f))
        if f.get("ended"):
            break
    return frames, time.time() - t0


def main(path=str(ROOT / "re/native_replay/ladder_last.json")):
    native = json.loads(Path(path).read_text(encoding="utf-8"))
    env = NativeEnv()
    a, ta = run(env, native, inject=False)
    b, tb = run(env, native, inject=True)
    env.close()
    print(f"一次性载入：{len(a)} 帧，{ta:.1f}s；逐条注入：{len(b)} 帧，{tb:.1f}s（每步一次往返）")
    first_diff = next((i for i, (x, y) in enumerate(zip(a, b)) if x != y), None)
    if first_diff is None and len(a) == len(b):
        print("完全一致 ✓")
    else:
        print(f"不一致：第一处差异在第 {first_diff} 帧（tick {a[first_diff][0] if first_diff is not None else '-'}）")
        if first_diff is not None:
            print("  载入:", a[first_diff])
            print("  注入:", b[first_diff])


if __name__ == "__main__":
    main(*sys.argv[1:])
