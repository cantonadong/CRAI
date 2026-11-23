"""下载 FirstLight HF 数据集（VanguardX101/IL_Replay）的 replays 分片，按 SHA256SUMS 校验，已下载且校验通过的跳过。

用法：python -u scripts/hf_download.py
"""
import hashlib
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DST = ROOT / "data/hf_il_replay"
BASE = "https://huggingface.co/datasets/VanguardX101/IL_Replay/resolve/main"


def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    (DST / "replays").mkdir(parents=True, exist_ok=True)
    sums_p = DST / "SHA256SUMS"
    urllib.request.urlretrieve(f"{BASE}/SHA256SUMS", sums_p)
    want = {}
    for line in sums_p.read_text().splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[1].lstrip("*").startswith("replays/"):
            want[parts[1].lstrip("*")] = parts[0].lower()
    print(f"{len(want)} 个 replays 分片", flush=True)
    for i, (rel, digest) in enumerate(sorted(want.items())):
        p = DST / rel
        if p.exists() and sha256(p) == digest:
            continue
        tmp = p.with_suffix(".part")
        urllib.request.urlretrieve(f"{BASE}/{rel}", tmp)
        if sha256(tmp) != digest:
            tmp.unlink()
            print(f"校验失败 {rel}", flush=True)
            continue
        tmp.replace(p)
        print(f"[{i + 1}/{len(want)}] {rel}", flush=True)
    print("完成", flush=True)


if __name__ == "__main__":
    main()
