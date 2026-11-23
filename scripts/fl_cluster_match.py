"""用"同一源文件的函数在二进制里相对位置大致不变"来匹配 FirstLight probe 的函数。

对一组旧偏移已知、开头字节已知的函数，在我们的 libg 里找一组命中，使它们之间的间距
和旧版间距最接近（允许整体平移和少量伸缩）。
"""
import itertools
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "refs/FirstLight_CR/native_runner/probe/cr_replay_probe.cpp"
LIBG = ROOT / "apk/libs/arm64-v8a/libg.so"

data = LIBG.read_bytes()
text = SRC.read_text(encoding="utf-8")
offsets = {k: int(v, 16) for k, v in re.findall(r"k(\w+?)Offset\s*=\s*(0x[0-9a-fA-F]+)", text)}
prologues = {
    k: bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", body))
    for k, body in re.findall(r"k(\w+?)Prologue\[\d+\]\s*=\s*\{([^}]*)\}", text, re.S)
}


def hits(bs):
    return [m.start() for m in re.finditer(re.escape(bs), data) if m.start() % 4 == 0]


CLUSTERS = [
    ["NativeReplayRunner", "ReplayBattleControllerLifecycle", "ReplayBattleControllerFullUpdate",
     "ReplayBattleControllerAlternateUpdate", "ReplayJsonSetter"],
    ["GameStateLoad", "GameStateStep"],
]

for cluster in CLUSTERS:
    names = [n for n in cluster if n in prologues]
    cands = {n: hits(prologues[n]) for n in names}
    base = names[0]
    best = []
    for anchor in cands[base]:
        shift = anchor - offsets[base]
        score, pick = 0, {base: anchor}
        for n in names[1:]:
            want = offsets[n] + shift
            near = min(cands[n], key=lambda h: abs(h - want), default=None)
            if near is None:
                score += 1 << 30
                continue
            pick[n] = near
            score += abs(near - want)
        best.append((score, shift, pick))
    best.sort(key=lambda t: t[0])
    print("cluster:", names)
    for score, shift, pick in best[:3]:
        print(f"  err={score:#x} shift={shift:+#x}")
        for n, h in pick.items():
            print(f"    {n:42s} old={offsets[n]:#09x} new={h:#09x} delta={h - offsets[n]:+#x}")
