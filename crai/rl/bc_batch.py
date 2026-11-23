"""行为克隆多局并排：把几局人类回放序列合并成一个多通道（lane）序列，一次前向同时算几局。

原版 distributed_expert_bc_update_v4 逐局、每 48 步串行前向，GPU 每次只算 1 局（每个局面约 21ms，
PPO 并排 30 局时每个局面约 0.8ms）。合并后 items 只剩一项，原函数不用改：
  - 时间维按最长一局补齐：补齐部分重复最后一帧，所有损失掩码清零（同 FirstLight collate_padded_il_sequences）；
  - 每步各局的单位/卡牌等动态数量不同：用 concatenate_padded_tensor_records 按 0（掩码为假）补齐；
  - 损失按全局有效数归一化（head_counts 对补齐帧计 0），所以与逐局累加梯度在数学上相同。
"""
from __future__ import annotations

from types import SimpleNamespace

import torch

from native_runner.training.v4.imitation import _IL_TENSOR_FIELDS, ILSequenceV4
from native_runner.training.v4.tensors import concatenate_padded_tensor_records


def collate_lanes(sequences: list[ILSequenceV4]) -> ILSequenceV4:
    for s in sequences:
        s.validate()
    T = max(s.time_steps for s in sequences)
    time_tensors = {}
    for name in _IL_TENSOR_FIELDS:
        values = [getattr(s, name) for s in sequences]
        if all(v is None for v in values):
            time_tensors[name] = None
            continue
        padded = []
        for s, v in zip(sequences, values):
            if v is None:
                v = torch.ones_like(s.valid_mask, dtype=torch.float32) if name == "frame_weight" else s.valid_mask
            if T > s.time_steps:
                v = torch.cat((v, v.new_zeros((T - s.time_steps, s.batch_size))))
            padded.append(v)
        time_tensors[name] = torch.cat(padded, dim=1)
    records = {
        name: tuple(
            concatenate_padded_tensor_records(tuple(getattr(s, name)[min(t, s.time_steps - 1)] for s in sequences))
            for t in range(T)
        )
        for name in ("observations", "actions")
    }
    out = ILSequenceV4(**records, **time_tensors, sequence_id="+".join(s.sequence_id or str(i) for i, s in enumerate(sequences)),
                       natural_time=all(s.natural_time for s in sequences))
    out.validate()
    return out


def merge_items(items: list) -> list:
    """sampler.sample() 的结果 → 只有一项的并排批次（交给 distributed_expert_bc_update_v4）。"""
    if len(items) <= 1:
        return items
    return [SimpleNamespace(sequence=collate_lanes([x.sequence for x in items]), metadata={"outcome": "win"},
                            supervised_hogs=sum(x.supervised_hogs for x in items),
                            burnin_frames=sum(x.burnin_frames for x in items))]
