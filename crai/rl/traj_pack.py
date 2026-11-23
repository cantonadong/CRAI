"""轨迹打包：把一局里逐步的 FirstLight 张量记录（每步几十个小张量）合并成每个字段一个大张量再存盘。

为什么（2026-11-22 实测）：一局约 1200 步、每步约 70 个小张量，torch.save/torch.load 要逐个重建张量对象，
主进程读一局要 3–9 秒（和其他线程抢 GIL 时更慢），30 局就是 3 分钟，卡在每次更新的关键路径上。
合并后每个字段只剩一个张量，读盘几乎只剩磁盘时间；读回时用切片视图还原成逐步记录，内容完全相同。

    packed = pack_steps(steps)        # 采集进程里
    steps = unpack_steps(packed)      # 主进程里
"""
from __future__ import annotations

from dataclasses import fields, is_dataclass

import torch

_RECORD_KEYS = ("obs", "act")


def _leaves(value, out: list):
    """按固定顺序收集叶子张量，返回结构模板（张量位置用其序号代替）。"""
    if isinstance(value, torch.Tensor):
        out.append(value)
        return ("T", len(out) - 1)
    if is_dataclass(value):
        return ("D", type(value), tuple((f.name, _leaves(getattr(value, f.name), out)) for f in fields(value)))
    return ("V", value)


def _build(tmpl, get):
    kind = tmpl[0]
    if kind == "T":
        return get(tmpl[1])
    if kind == "D":
        return tmpl[1](**{name: _build(sub, get) for name, sub in tmpl[2]})
    return tmpl[1]


def _pack_record(values: list):
    """同一类记录（各步的 obs 或 act）→ (模板列表, 每个叶子位置的合并张量或逐步张量列表)。
    结构不同的步单独存模板（极少见）；同一叶子位置形状一致时沿第 0 维拼接。"""
    tmpls, leaves = [], []
    for v in values:
        out: list = []
        tmpls.append(_leaves(v, out))
        leaves.append(out)
    same = all(t == tmpls[0] for t in tmpls) if tmpls else True
    if not same:
        return {"mode": "raw", "values": values}
    n = len(leaves[0]) if leaves else 0
    cols = []
    for i in range(n):
        col = [step[i] for step in leaves]
        shapes = {tuple(t.shape[1:]) for t in col}
        if len(shapes) == 1 and all(t.ndim >= 1 and t.shape[0] == col[0].shape[0] for t in col):
            cols.append(("cat", int(col[0].shape[0]), torch.cat([t.detach().contiguous() for t in col], 0)))
        else:
            cols.append(("list", 0, col))
    return {"mode": "packed", "tmpl": tmpls[0], "cols": cols, "n": len(values)}


def _unpack_record(p) -> list:
    if p["mode"] == "raw":
        return p["values"]
    tmpl, cols, n = p["tmpl"], p["cols"], p["n"]
    out = []
    for s in range(n):
        def get(i, s=s):
            kind, b, t = cols[i]
            return t[s * b:(s + 1) * b] if kind == "cat" else t[s]
        out.append(_build(tmpl, get))
    return out


def pack_steps(steps: list[dict]) -> dict:
    if not steps or any(set(s) - set(steps[0]) or set(steps[0]) - set(s) for s in steps):
        return {"packed": False, "steps": steps}
    rec = {k: _pack_record([s[k] for s in steps]) for k in _RECORD_KEYS if k in steps[0]}
    rest = [{k: v for k, v in s.items() if k not in rec} for s in steps]
    return {"packed": True, "rec": rec, "rest": rest}


def unpack_steps(p: dict) -> list[dict]:
    if not p.get("packed"):
        return p["steps"]
    cols = {k: _unpack_record(v) for k, v in p["rec"].items()}
    return [dict(r, **{k: cols[k][i] for k in cols}) for i, r in enumerate(p["rest"])]
