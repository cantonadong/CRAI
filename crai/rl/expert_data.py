"""人类回放 → FirstLight 行为克隆序列（ILSequenceV4）。

输入：新代理库重演的逐 tick 帧（data/resim_headless/hog_v2/<tag>.jsonl.gz）+ 第一轮的原生 JSON（人类指令，
data/resim_headless/hog/<tag>.native.json.gz）。只学野猪方（hog_owner）。

做法（与实战完全相同的观测管线，保证训练与实战一致）：
  - 用 FLAgent.prepare 逐帧喂给适配器（EngineBattleEnv），在每个决策点（第 90 tick 起每 5 tick）得到观测；
  - 张量化观测；人类在 [决策 tick, +5) 内下达的指令（ct=153 出牌、ct=189 技能，按下达 tick t）
    用 FirstLight 的 build_expert_action_batch 对齐到候选，得到动作标签；
  - 把人类动作记进张量化器的动作历史，出牌登记为在途牌（与实战 Everlight 的在途特征一致）；
  - 对不齐的窗口（超过 2 个动作、候选匹配不到等）标为无效，不参与损失，但记 WAIT 保持历史连续。
奖励/价值不用（BC 只训练策略头）。
"""
from __future__ import annotations

import gzip
import json
import uuid
from pathlib import Path

import torch

from crai.env import fl_agent as F
from native_runner.contracts import ActionKind, ActionV1, TargetKind
from native_runner.training.v4.expert import (
    ExpertActionAlignmentError, TimedExpertActionV4, build_expert_action_batch,
)
from native_runner.training.v4.imitation import ILSequenceV4
from native_runner.training.v4.tensors import concatenate_tensor_records  # noqa: F401  (保留给批量拼接)

ROOT = Path(__file__).resolve().parents[2]
FRAMES = ROOT / "data/resim_headless/hog_v2"
NATIVE = ROOT / "data/resim_headless/hog"
FORMS_BY_OWNER_KEY = "forms"


def _load(tag: str):
    with gzip.open(FRAMES / f"{tag}.jsonl.gz", "rt", encoding="utf-8") as f:
        frames = [json.loads(x) for x in f if x.strip()]
    native = json.loads(gzip.decompress((NATIVE / f"{tag}.native.json.gz").read_bytes()))
    return frames, native


def _forms(native: dict, side: int) -> dict[int, int]:
    """原生 JSON 卡组里的形态（el）。"""
    deck = native["battle"].get(f"deck{side}", {}).get("sp", [])
    return {int(c["d"]): int(c.get("el", 0)) for c in deck if c.get("el")}


class _Teacher(F.FLAgent):
    """只用适配器和张量化器，不跑模型决策。"""

    def _new_session(self, tz):
        dev = str(next(self.loaded.model.parameters()).device)
        return F.PolicySessionV4(self.loaded.model, tz, device=dev, sample=False)


def build_sequence(tag: str, owner: int, loaded) -> tuple[ILSequenceV4 | None, dict]:
    frames, native = _load(tag)
    forms = {s: _forms(native, s) for s in (0, 1)}
    t = _Teacher(owner, forms, model=loaded, device="cpu")
    acct = next(p["acct"] for p in frames[0]["players"] if p and p["side"] == owner)
    cmds = [c for c in native["cmd"] if c["c"].get("idLo") == acct and c["ct"] in (153, 189)]
    obs_list, act_list, valid = [], [], []
    stats = {"decisions": 0, "act_windows": 0, "labels": 0, "misaligned": 0, "dropped_early": 0}
    ci = 0
    while ci < len(cmds) and cmds[ci]["c"]["t"] < F.FIRST_POLICY_DECISION_TICK:
        ci += 1
        stats["dropped_early"] += 1
    for frame in frames:
        obs = t.prepare(frame, acct)
        if obs is None:
            continue
        tick = int(frame["tick"])
        tz = t.session.tensorizer
        hb = tz.tensorize(obs, validate=False)
        window_cmds = []
        while ci < len(cmds) and cmds[ci]["c"]["t"] < tick + F.POLICY_DECISION_TICKS:
            if cmds[ci]["c"]["t"] >= tick:
                window_cmds.append(cmds[ci])
            ci += 1
        me = next(p for p in frame["players"] if p and p["side"] == owner)
        window, plays = [], []
        ok = True
        for k, c in enumerate(window_cmds):
            cc = c["c"]
            if c["ct"] == 153:
                d = ((cc["sel"]["pd"] >> 22) & 0x3F) - 1
                if d not in me["hand"]:
                    ok = False
                    break
                slot = me["hand"].index(d)
                gx, gy = cc["px"] // 1000, cc["py"] // 1000
                off = ((cc["px"] - gx * 1000 - 500) / 1000.0, (cc["py"] - gy * 1000 - 500) / 1000.0)
                a = ActionV1(owner=owner, kind=ActionKind.PLAY_CARD, hand_slot=slot, card_id=int(cc["sel"]["os"]),
                             target_kind=TargetKind.GRID, target_grid=(gx, gy), subcell_offset=off,
                             action_id=str(uuid.uuid4()))
                plays.append((a, int(cc["sel"]["os"]), slot, d, int(cc["sel"]["pd"]), int(cc["t"]), int(cc["t2"])))
            else:
                src = None
                ctx = t.env._rich_context(t.env._raw)
                for key, cid in ctx.canonical_ids.items():
                    if int(key[2]) == int(cc["cgid"]):
                        src = cid
                        break
                a = ActionV1(owner=owner, kind=ActionKind.ACTIVATE_ABILITY, source_entity=src,
                             action_id=str(uuid.uuid4()))
            window.append(TimedExpertActionV4(source_tick=int(cc["t"]), action=a, source_index=k))
        acts = None
        if ok:
            try:
                acts = build_expert_action_batch(hb, [window], decision_ticks_by_row=[tick],
                                                 perspectives=[tz.perspective], config=tz.config, validate=False)
            except ExpertActionAlignmentError:
                acts = None
        if acts is None:
            stats["misaligned"] += 1
            acts = build_expert_action_batch(hb, [()], decision_ticks_by_row=[tick], perspectives=[tz.perspective],
                                             config=tz.config, validate=False)
            valid.append(False)
        else:
            valid.append(True)
            stats["labels"] += len(window)
            stats["act_windows"] += bool(window)
        tz.record_action(acts, hb, row=0, validate=False)
        t.session._first_decision = False
        for a, card_id, slot, d, pd, ti, t2 in plays:
            t._note_pending(a, card_id, slot, d, pd, ti, t2)
            t.plays += 1
        obs_list.append(hb)
        act_list.append(acts)
        stats["decisions"] += 1
    if not obs_list:
        return None, stats
    n = len(obs_list)
    vm = torch.tensor(valid, dtype=torch.bool).reshape(n, 1)
    es = torch.zeros((n, 1), dtype=torch.bool)
    es[0, 0] = True
    seq = ILSequenceV4(observations=tuple(obs_list), actions=tuple(act_list), episode_start=es, valid_mask=vm,
                       returns=torch.zeros((n, 1)), gate_loss_mask=vm.clone(), value_loss_mask=torch.zeros_like(vm),
                       sequence_id=tag)
    return seq, stats


# ---------------------------------------------------------------------------
# 行为克隆采样器（给 FirstLight 的 distributed_expert_bc_update_v4 用）
# ---------------------------------------------------------------------------
SUMMARY = ROOT / "data/resim_headless/hog_v2_summary.jsonl"


class ExpertSampler:
    """每次随机取 n 局"野猪方赢了"的人类对局，现场构建 ILSequenceV4（每局约 8 秒，序列约 120MB，不存盘）。"""

    def __init__(self, loaded, n: int = 8, winners_only: bool = True, seed: int = 0):
        import random
        self.loaded = loaded
        self.n = n
        self.rng = random.Random(seed)
        rows = [json.loads(x) for x in SUMMARY.read_text(encoding="utf-8").splitlines() if x.strip()]
        if winners_only:
            rows = [r for r in rows if r["resim"][str(r["hog_owner"])] > r["resim"][str(1 - r["hog_owner"])]]
        self.rows = rows
        self.last_stats: list[dict] = []

    def sample(self, update_step: int, rank: int = 0, world_size: int = 1) -> list:
        from types import SimpleNamespace
        # 汇总文件在后台重演中持续增长：每次重新读，新完成的局也能被抽到
        rows = [json.loads(x) for x in SUMMARY.read_text(encoding="utf-8").splitlines() if x.strip()]
        rows = [r for r in rows if r["resim"][str(r["hog_owner"])] > r["resim"][str(1 - r["hog_owner"])]] or self.rows
        items, self.last_stats = [], []
        tries = 0
        while len(items) < self.n and tries < self.n * 3:
            tries += 1
            r = self.rng.choice(rows)
            try:
                seq, st = build_sequence(r["tag"], int(r["hog_owner"]), self.loaded)
            except Exception as e:  # noqa: BLE001 —— 个别对局适配器报错就换一局
                self.last_stats.append({"tag": r["tag"], "error": f"{type(e).__name__}: {e}"})
                continue
            if seq is None or not bool(seq.valid_mask.any()):
                continue
            self.last_stats.append({"tag": r["tag"], **st})
            from native_runner.training.v4.ppo_expert_bc import hog_action_rows
            items.append(SimpleNamespace(sequence=seq, metadata={"tag": r["tag"], "outcome": "win"},
                                         supervised_hogs=len(hog_action_rows(seq)), burnin_frames=0))
        return items
