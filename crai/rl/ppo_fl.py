"""在我们的离线引擎上对 FirstLight V4 模型做 PPO（yesdelay 分支：下牌 20 tick 后生效，与实战一致）。

只做"单机、按局顺序采集"的最小实现，PPO 更新本身直接复用 FirstLight 的 distributed_ppo_update_v4
（单进程时 world_size = 1）。每局 = 一条 lane；每 5 tick 一个决策步。

奖励（与 FirstLight 主线野猪专精的配方一致：胜负 + 塔血势能塑形 + 圣水溢出惩罚，不加野猪专项奖励）：
  - 终局：胜 +1 / 负 -1 / 平 0（按皇冠）；
  - 塔血势能：shaping_beta × Δ(敌方塔损失 − 己方塔损失)，损失 = 公主塔各占 1/3、国王塔 1/3（按血量比例）；
  - 圣水溢出：整局浪费的圣水超过 grace 后，每浪费 1 点扣 overflow_penalty。

用法见 scripts/ppo_train.py。必须在 CRAI_FL_ROOT=yesdelay 下运行。
"""

from __future__ import annotations

import os
import time
from dataclasses import dataclass, field

import torch

from crai.env import fl_agent as F
from crai.env.match import is_tower

if not F.YES_DELAY:
    raise RuntimeError("PPO 需要 yesdelay 分支：先设置环境变量 CRAI_FL_ROOT=yesdelay")

from native_runner.training.v4.decoding import decode_action_sequence_v4  # noqa: E402
from native_runner.training.v4.distributed_ppo import distributed_ppo_update_v4  # noqa: E402
from native_runner.training.v4.policy_session import PolicyDecisionV4, PolicySessionV4  # noqa: E402
from native_runner.training.v4.ppo import PPOConfigV4, StoredRolloutV4  # noqa: E402
from native_runner.training.v4.tensors import (  # noqa: E402
    concatenate_padded_tensor_records, concatenate_tensor_records,
)

KING_X = 9000


# ---------------------------------------------------------------------------
# 采集：学习方会话（采样 + 记录）
# ---------------------------------------------------------------------------
class RecordingSession(PolicySessionV4):
    """与 PolicySessionV4.decide 相同的流程，但总是按 PPO 采样，并把每个决策步的观测、动作、log π、价值存下来。
    不使用 force_act（强制出牌会改变动作分布，PPO 里不要）。"""

    def __init__(self, model, tensorizer, *, device, agent: "PPOLearner"):
        super().__init__(model, tensorizer, device=device, sample=True)
        self._cuda_graph = None
        self.agent = agent
        self.steps: list[dict] = []

    @torch.inference_mode()
    def decide(self, observation, *, force_act: bool = False):
        host_batch = self.tensorizer.tensorize(observation, validate=False)
        episode_start = torch.tensor([self._first_decision], dtype=torch.bool, device=self.device)
        output = self.model.sample_for_ppo_rollout(host_batch.to_model_input(self.device), self.state,
                                                   episode_start=episode_start, validate=False)
        output.actions.validate(self.tensorizer.config, candidate_count=int(host_batch.candidates.mask.shape[1]))
        decoded = decode_action_sequence_v4(
            output.actions, host_batch.candidates, row=0, observation=observation, catalog=self.tensorizer.catalog,
            deck=self.tensorizer.deck, card_costs=self.tensorizer.card_costs,
            ability_id_by_vocab_id=self.tensorizer.ability_id_by_vocab_id,
            horizontal_mirror=self.tensorizer.perspective.horizontal_mirror,
            hand_slot_permutation=self.tensorizer.perspective.hand_slot_permutation,
            config=self.tensorizer.config, base_latency_ticks=1, base_latency_ms=0.0, validate=False)
        self.tensorizer.record_action(output.actions, host_batch, row=0, validate=False)
        self.record(host_batch, output.actions, float(output.log_prob.reshape(-1)[0]),
                    float(output.value.reshape(-1)[0]))
        self.state = output.next_state
        self._first_decision = False
        return PolicyDecisionV4(decoded=decoded, inference_ms=0.0)

    def record(self, host_batch, actions, logp: float, value: float):
        """存一个决策步（批量推理路径 fl_agent.batch_decide 也调用这里）。"""
        self.steps.append({
            "obs": host_batch,                                   # 已在 CPU
            "act": actions.to("cpu"),
            "logp": logp,
            "value": value,
            "info": self.agent.last_info,
            "owner": self.agent.owner,
        })


def frame_info(raw: dict, owner: int) -> dict:
    """奖励要用的局面摘要：双方塔血比例、己方圣水。"""
    towers = {0: {}, 1: {}}
    for e in raw["entities"]:
        if is_tower(e) and e["max_hp"] > 0:
            kind = "king" if e["x"] == KING_X else ("left" if e["x"] < KING_X else "right")
            towers[e["side"]][kind] = max(0, e["hp"]) / e["max_hp"]
    me = next((p for p in raw["players"] if p and p["side"] == owner), None)
    return {"tick": int(raw["tick"]), "towers": towers, "elixir": float(me["elixir"]) if me else 0.0}


def tower_loss(towers: dict) -> float:
    """一方塔的损失（0..1）：两座公主塔和国王塔各占 1/3；被摧毁（帧里消失）的塔按损失 1 计。"""
    return sum(1.0 - towers.get(k, 0.0) for k in ("left", "right", "king")) / 3.0


class PPOLearner(F.FLAgent):
    """学习方：决策时先记下局面摘要，再交给 RecordingSession 采样。"""

    def __init__(self, owner, forms, loaded, device):
        super().__init__(owner, forms, model=loaded, sample=True, device=device)
        self.last_info = None

    def _new_session(self, tz):
        return RecordingSession(self.loaded.model, tz, device=self.device, agent=self)

    def prepare(self, frame, account_lo):
        self.last_info = frame_info(frame, self.owner)
        return super().prepare(frame, account_lo)


# ---------------------------------------------------------------------------
# 奖励
# ---------------------------------------------------------------------------
@dataclass
class RewardConfig:
    terminal: float = 1.0
    shaping_beta: float = 0.05
    overflow_penalty: float = 0.01        # 每浪费 1 点圣水（超过 grace 之后）
    overflow_grace: float = 4.0


def elixir_rate(tick: int) -> float:
    """每 tick 回复的圣水（常规 2.8 秒 1 点；2 分钟后双倍；加时最后 1 分钟三倍，按近似处理）。"""
    mult = 1 if tick < 2400 else (2 if tick < 4800 else 3)
    return mult / 56.0


def episode_rewards(steps: list[dict], my_crowns: int, opp_crowns: int, cfg: RewardConfig) -> tuple[list[float], dict]:
    owner_infos = [s["info"] for s in steps]
    n = len(steps)
    rewards = [0.0] * n
    wasted = 0.0
    overflow_total = 0.0
    me = steps[0]["owner"] if steps else 0
    opp = 1 - me
    for t in range(n):
        cur = owner_infos[t]
        nxt = owner_infos[t + 1] if t + 1 < n else None
        if nxt is not None:
            phi_now = tower_loss(cur["towers"][opp]) - tower_loss(cur["towers"][me])
            phi_next = tower_loss(nxt["towers"][opp]) - tower_loss(nxt["towers"][me])
            rewards[t] += cfg.shaping_beta * (phi_next - phi_now)
            if cur["elixir"] >= 9.99 and nxt["elixir"] >= 9.99:
                w = (nxt["tick"] - cur["tick"]) * elixir_rate(cur["tick"])
                before = max(0.0, wasted - cfg.overflow_grace)
                wasted += w
                pen = cfg.overflow_penalty * (max(0.0, wasted - cfg.overflow_grace) - before)
                rewards[t] -= pen
                overflow_total += pen
    outcome = (my_crowns > opp_crowns) - (my_crowns < opp_crowns)
    if n:
        rewards[-1] += cfg.terminal * outcome
    return rewards, {"wasted_elixir": round(wasted, 2), "overflow_penalty": round(overflow_total, 4),
                     "outcome": outcome}


# ---------------------------------------------------------------------------
# 组装 StoredRolloutV4 并更新
# ---------------------------------------------------------------------------
@dataclass
class Episode:
    steps: list[dict]
    rewards: list[float]
    meta: dict = field(default_factory=dict)


def build_rollout(episodes: list[Episode], model, policy_state_id: str) -> StoredRolloutV4:
    lanes = [e for e in episodes if e.steps]
    B = len(lanes)
    T = max(len(e.steps) for e in lanes)
    obs_seq, act_seq = [], []
    for t in range(T):
        # 已结束的 lane 用最后一步填充（valid=False，不参与损失）
        obs_seq.append(concatenate_padded_tensor_records(tuple(e.steps[min(t, len(e.steps) - 1)]["obs"] for e in lanes)))
        act_seq.append(concatenate_tensor_records(tuple(e.steps[min(t, len(e.steps) - 1)]["act"] for e in lanes)))
    z = lambda: torch.zeros((T, B), dtype=torch.float32)  # noqa: E731
    b = lambda: torch.zeros((T, B), dtype=torch.bool)  # noqa: E731
    episode_start, valid, terminated, truncated = b(), b(), b(), b()
    logp, value, next_value, rewards = z(), z(), z(), z()
    for i, e in enumerate(lanes):
        n = len(e.steps)
        episode_start[0, i] = True
        valid[:n, i] = True
        terminated[n - 1, i] = True
        for t, s in enumerate(e.steps):
            logp[t, i] = s["logp"]
            value[t, i] = s["value"]
            next_value[t, i] = e.steps[t + 1]["value"] if t + 1 < n else 0.0
            rewards[t, i] = e.rewards[t]
    return StoredRolloutV4(
        observations=tuple(obs_seq), actions=tuple(act_seq), episode_start=episode_start, valid_mask=valid,
        behavior_log_prob=logp, behavior_value=value, next_value=next_value, rewards=rewards,
        terminated=terminated, truncated=truncated, initial_state=model.initial_state(B, device="cpu"),
        gate_temperature=float(model.ppo_gate_temperature), action_temperature=float(model.ppo_action_temperature),
        continue_temperature=float(model.ppo_continue_temperature), policy_state_id=policy_state_id)


def ppo_update(model, optimizer, rollout: StoredRolloutV4, *, epochs: int = 2, chunk: int = 48,
               cfg: PPOConfigV4 | None = None) -> list[dict]:
    model.train()
    try:
        metrics = distributed_ppo_update_v4(
            model, optimizer, rollout, epochs=epochs, lane_minibatch_size=rollout.batch_size,
            lane_microbatch_size=max(1, min(rollout.batch_size, int(os.environ.get("CRAI_PPO_MICRO", "4")))),
            sequence_chunk_steps=chunk,
            config=cfg or PPOConfigV4(gamma=0.9997, gae_lambda=0.98, clip_range=0.1, value_clip_range=0.1,
                                      target_kl=0.01),
            autocast_dtype=torch.bfloat16 if next(model.parameters()).is_cuda else None)
    finally:
        model.eval()
    return metrics


def timestamp() -> str:
    return time.strftime("%H:%M:%S")
