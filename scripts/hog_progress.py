"""查看 hf_resim_hog.py 的进度：完成局数、成功率、结局一致率、按形态分组统计、预计剩余时间。

用法：python scripts/hog_progress.py
"""
import collections
import json
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SUMMARY = ROOT / "data/resim_headless/hog_summary.jsonl"
EST_TOTAL = 10900  # 按第 1 个分片估算：每 5000 局约 210 方野猪 2.6，共 52 个分片


def main():
    rows = [json.loads(line) for line in SUMMARY.read_text(encoding="utf-8").splitlines() if line.strip()]
    ok = [r for r in rows if "error" not in r]
    print(f"已处理 {len(rows)}（约 {len(rows) / EST_TOTAL:.0%}），成功 {len(ok)}，失败 {len(rows) - len(ok)}")
    if ok:
        print(f"皇冠完全一致 {sum(r['exact'] for r in ok)}/{len(ok)}，胜负一致 {sum(r['same_winner'] for r in ok)}/{len(ok)}")
        by_form = collections.defaultdict(list)
        for r in ok:
            f = r["forms"][str(r["hog_owner"])] if r.get("hog_owner") is not None else {}
            by_form[tuple(sorted(f"{k}:{v}" for k, v in f.items()))].append(r["exact"])
        print("野猪方形态（el 1=进化 2=英雄）：")
        for k, v in sorted(by_form.items(), key=lambda kv: -len(kv[1]))[:6]:
            print(f"  {len(v):5d} 局，一致 {sum(v) / len(v):.0%}  {', '.join(k)}")
    errs = collections.Counter(r["error"][:70] for r in rows if "error" in r)
    for e, n in errs.most_common(3):
        print(f"  错误 {n}：{e}")
    st = SUMMARY.stat().st_mtime
    print(f"最后写入 {time.strftime('%H:%M:%S', time.localtime(st))}（{time.time() - st:.0f} 秒前）")


if __name__ == "__main__":
    main()
