"""把一份游戏原生回放 JSON 交给游戏内的离线引擎（crai/probe/proxy_min.c）重演，取回逐 tick 数据。

前提：代理库已部署（scripts/probe_min.ps1 deploy）且游戏在运行。
用法：python scripts/headless_resim.py <native.json> [输出.jsonl]
输出默认 data/resim_headless/<名字>.jsonl，并打印结局（双方剩余塔数、皇冠）。
"""
import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.device import ADB, SERIAL, as_root, instance_of  # noqa: E402

TOWER_POS = {(9000, 3000), (3500, 6500), (14500, 6500), (9000, 29000), (3500, 25500), (14500, 25500)}
PKG = "nullsroyale.rel.free"
FILES = f"/data/data/{PKG}/files"


LDCONSOLE = r"D:\Program\LDPlayer9\ldconsole.exe"


def sh(cmd: str, timeout: float = 30) -> str:
    try:
        return subprocess.run([ADB, "-s", SERIAL, "shell", cmd], capture_output=True, text=True,
                              timeout=timeout).stdout
    except subprocess.TimeoutExpired:
        return ""


def device_ok() -> bool:
    try:
        out = subprocess.run([ADB, "devices"], capture_output=True, text=True, timeout=15).stdout
    except subprocess.TimeoutExpired:
        return False
    return any(line.startswith(SERIAL) and line.endswith("device") for line in out.splitlines())


def block_gamepad() -> None:
    """模拟器重启后重新屏蔽手柄（见 scripts/block_gamepad.ps1、crai/tools/grab_joystick.c）。"""
    if sh("ls /data/local/tmp/grab_joystick").strip():
        sh(as_root("pkill grab_joystick; nohup /data/local/tmp/grab_joystick > /data/local/tmp/grab_joystick.log 2>&1 &"))


def ensure_device() -> None:
    """设备不在（模拟器虚拟机挂了）就用 ldconsole 重启实例 0，等系统启动完成。"""
    if device_ok() and sh("getprop sys.boot_completed").strip() == "1":
        return
    idx = str(instance_of(SERIAL))          # 只重启自己用的那个实例，不碰其他实例（例如天梯实例 0）
    print(f"设备不可用，重启 LDPlayer 实例 {idx}", flush=True)
    subprocess.run([LDCONSOLE, "quit", "--index", idx], capture_output=True, timeout=60)
    time.sleep(8)
    subprocess.run([LDCONSOLE, "launch", "--index", idx], capture_output=True, timeout=60)
    t0 = time.time()
    while time.time() - t0 < 300:
        time.sleep(5)
        if device_ok() and sh("getprop sys.boot_completed").strip() == "1":
            time.sleep(10)
            block_gamepad()
            return
    raise RuntimeError("LDPlayer 重启后 5 分钟内没有起来")


def su(cmd: str) -> str:
    return sh(as_root(cmd))


def summarize(frames: list[dict]) -> dict:
    last = frames[-1]
    towers = {0: 0, 1: 0}
    for e in last["entities"]:
        # 按塔的固定坐标认（card == -1 的还有对局中途生成的实体，见 crai/env/match.py: TOWER_POS）
        if e["card"] == -1 and (e["x"], e["y"]) in TOWER_POS and e["hp"] > 0:
            towers[e["side"]] += 1
    king = {p["side"]: p["king_hp"][0] for p in last["players"] if p}
    crowns = {}
    for side in (0, 1):
        other = 1 - side
        crowns[side] = 3 if king.get(other, 1) <= 0 else 3 - towers[other]
    return {"ticks": last["tick"], "frames": len(frames), "towers_alive": towers, "king_hp": king,
            "crowns": crowns, "max_entities": max(len(f["entities"]) for f in frames)}


def ensure_game() -> None:
    """设备和游戏都要在：设备不在就重启模拟器；游戏没在运行就启动它，并等代理库就绪（logcat 出现 headless: ready）。"""
    ensure_device()
    if sh(f"pidof {PKG}").strip():
        return
    subprocess.run([ADB, "-s", SERIAL, "logcat", "-c"], capture_output=True, timeout=30)
    sh(f"monkey -p {PKG} -c android.intent.category.LAUNCHER 1")
    t0 = time.time()
    while time.time() - t0 < 90:
        try:
            log = subprocess.run([ADB, "-s", SERIAL, "logcat", "-d", "-s", "CRProbe"], capture_output=True, text=True,
                                 timeout=30).stdout
        except subprocess.TimeoutExpired:
            log = ""
        if "headless: ready" in log:
            time.sleep(2)
            return
        time.sleep(2)
    raise RuntimeError("游戏启动后 90 秒内代理库没有就绪")


def main(src: str, out: str | None = None, quiet: bool = False) -> dict:
    ensure_game()
    src_p = Path(src)
    out_p = Path(out) if out else ROOT / "data/resim_headless" / (src_p.stem + ".jsonl")
    out_p.parent.mkdir(parents=True, exist_ok=True)
    uid = sh(f"stat -c %u {FILES}").strip() or su(f"stat -c %u {FILES}").strip()
    subprocess.run([ADB, "-s", SERIAL, "push", str(src_p), "/data/local/tmp/crprobe_in.json"], capture_output=True,
                   timeout=60)
    su(f"rm -f {FILES}/crprobe_done {FILES}/crprobe_out.jsonl; cp /data/local/tmp/crprobe_in.json {FILES}/crprobe_in.tmp; "
       f"chown {uid}:{uid} {FILES}/crprobe_in.tmp; mv {FILES}/crprobe_in.tmp {FILES}/crprobe_in.json")
    t0 = time.time()
    while "crprobe_done" not in su(f"ls {FILES}"):
        if time.time() - t0 > 120:
            raise TimeoutError("离线引擎 2 分钟没有结束")
        if not sh(f"pidof {PKG}").strip():
            raise RuntimeError("游戏进程没了（可能崩溃）")
        time.sleep(0.2)
    elapsed = time.time() - t0
    su(f"cp {FILES}/crprobe_out.jsonl /data/local/tmp/crprobe_out.jsonl; chmod 644 /data/local/tmp/crprobe_out.jsonl")
    subprocess.run([ADB, "-s", SERIAL, "pull", "/data/local/tmp/crprobe_out.jsonl", str(out_p)], capture_output=True,
                   timeout=120)
    frames = [json.loads(line) for line in out_p.read_text(encoding="utf-8").splitlines() if line.strip()]
    if not frames:
        raise RuntimeError("没有输出帧，看 logcat -s CRProbe")
    s = summarize(frames)
    s["seconds"] = round(elapsed, 1)
    s["out"] = str(out_p)
    if not quiet:
        print(json.dumps(s, ensure_ascii=False))
    return s


if __name__ == "__main__":
    main(*sys.argv[1:])
