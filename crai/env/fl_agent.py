"""FirstLight_CR V4 模型（野猪 2.6 专精等）接入我们的离线引擎。

做法见 docs/fl_raw_schema.md：
  - 我们的引擎帧 → FirstLight probe 格式的 raw（手牌带 pd/cost，单位带 dataGlobalId，塔按 TOWER_LAYOUT 坐标）；
  - 子类化 BattleEnvV1：不连接 native，rich telemetry 合成为"全空但结构合法"；
  - 复刻 reset/step 中与 native 无关的步骤，自己补双方出牌的 action_executed 事件；
  - tick < 90 只更新 tracker，之后每 5 tick decide 一次；ActionV1 → 原生出牌指令（ct=153）。
FirstLight_CR 为 Apache-2.0（refs/FirstLight_CR），这里只调用其代码。

在 crai.env.match.play_match 里作为 AI 使用：agent = FLAgent(owner, deck_specs)；它实现 decide_raw(raw) -> [原生指令]。
"""

from __future__ import annotations

import gzip
import hashlib
import json
import os
import sys
import time
import uuid
from pathlib import Path
from types import MappingProxyType

ROOT = Path(__file__).resolve().parents[2]
# 选择 FirstLight 分支：main（野猪专精等，下牌按约 1 tick 生效训练）或 yesdelay（Everlight，按 20 tick 延迟训练）。
# 两个分支的包同名，一个进程只能用一个：设置环境变量 CRAI_FL_ROOT=yesdelay 切换。
FL = ROOT / "refs" / ("FirstLight_CR_yesdelay" if os.environ.get("CRAI_FL_ROOT") == "yesdelay" else "FirstLight_CR")
sys.path.insert(0, str(FL))
YES_DELAY = FL.name.endswith("yesdelay")

from native_runner.battle_env import BattleEnvV1  # noqa: E402
from native_runner.contracts import ActionKind  # noqa: E402
from native_runner.match_factory import MatchConfig  # noqa: E402
from native_runner.rich_telemetry_adapter import (  # noqa: E402
    ABILITY_BUTTON_STATE_LABELS, ABILITY_QUEUEABLE_BUTTON_STATES, RichAbilityRuntimeTelemetry,
    RichCombatEventEnvelope, RichObjectTelemetry, RichPlayerRuntimeTelemetry, RichTelemetrySnapshot,
    RuntimeEffectCatalog, normalized_ability_cooldown_ms,
)
from native_runner.training.v4.expert import FIRST_POLICY_DECISION_TICK, POLICY_DECISION_TICKS  # noqa: E402
from native_runner.training.v4.factory import build_episode_tensorizer_v4, production_semantic_bundle  # noqa: E402
from native_runner.training.v4.policy_session import PolicySessionV4, load_policy_v4  # noqa: E402

from crai.data import cards  # noqa: E402

CHECKPOINTS = {
    "hog1": FL / "checkpoints/2_6hog_expert/hog26-specialist1.pt",
    "hog2": FL / "checkpoints/2_6hog_expert/hog26-specialist2.pt",
    "general": FL / "checkpoints/General/checkpoint-step-00000460.pt",
    "il": FL / "checkpoints/IL/checkpoint-step-00029396.pt",
    "everlight": FL / "checkpoints/Everlight/v15-general-il100-kl5e-2-v80.pt",   # 只在 yesdelay 分支
}
if YES_DELAY:
    from native_runner.battle_env import _PendingRecord  # noqa: E402
    from native_runner.cr_native_env import COMMAND_EXECUTION_DELAY_TICKS  # noqa: E402
MIRROR_CARD_ID = 28000006
TOWER_POS = {(9000, 3000), (3500, 6500), (14500, 6500), (9000, 29000), (3500, 25500), (14500, 25500)}
RULESET_ID = hashlib.sha256(b"crai-engine-16.635.1-bridge.v1").hexdigest()
COMMAND_AGE = 20

_BUNDLE = None
_KNOWN_DGID: set[int] | None = None
_MODELS: dict[str, object] = {}


def _bundle():
    global _BUNDLE
    if _BUNDLE is None:
        _BUNDLE = production_semantic_bundle()
    return _BUNDLE


def known_dgids() -> set[int]:
    """FirstLight 模型认识的单位原型（数据全局 ID）；不认识的对象必须过滤掉，否则 tensorizer 会报错。"""
    global _KNOWN_DGID
    if _KNOWN_DGID is None:
        cat = json.load(gzip.open(FL / "native_runner/data/competitive/model_catalogs.json.gz", "rt",
                                  encoding="utf-8"))["catalogs"]["entity_archetype_catalog"]
        s = {int(g) for g, _ in cat["character_global_ids"]}
        for key in cat["archetype_keys"]:
            num = key.partition(":")[2]
            if num.lstrip("-").isdigit():
                s.add(int(num))
        _KNOWN_DGID = s
    return _KNOWN_DGID


def load_model(name: str):
    if name not in _MODELS:
        _MODELS[name] = load_policy_v4(str(CHECKPOINTS.get(name, name)), device="cpu")
    return _MODELS[name]


# ---------------------------------------------------------------------------
# 我们的引擎帧 → FirstLight raw
# ---------------------------------------------------------------------------
def to_fl_raw(frame: dict, generation: int = 1) -> dict:
    known = known_dgids()
    players = []
    for p in frame["players"]:
        if not p:
            continue
        deck = p["deck"]
        hand = []
        for i, (d, pd) in enumerate(zip(p["hand"], p.get("pd", [0, 0, 0, 0]))):
            if not (0 <= d < 8) or not pd:
                continue
            hand.append({"handIndex": i, "deckSlot": d, "cardId": deck[d], "commandCardId": deck[d],
                         "cardParameter": int(pd), "cost": int(pd) >> 28})
        queue = [q for q in p["next"] if 0 <= q < 8]
        in_hand = {h["deckSlot"] for h in hand}
        cycle_slots = [q for q in queue if q not in in_hand]
        for s in range(8):                    # hand + cycle 必须恰好覆盖 8 张
            if s not in in_hand and s not in cycle_slots:
                cycle_slots.append(s)
        players.append({
            "owner": p["side"], "accountId": p.get("acct", 0), "elixirRaw": int(round(p["elixir"] * 10000)),
            "crownsRaw": 0, "hand": hand,
            "cycle": [{"cycleIndex": k, "deckSlot": s, "cardId": deck[s]} for k, s in enumerate(cycle_slots)],
            "nextCard": ({"deckSlot": cycle_slots[0], "cardId": deck[cycle_slots[0]]} if cycle_slots else None),
            "deck": [{"deckSlot": s, "cardId": deck[s]} for s in range(8)],
        })
    objects = []
    for e in frame["entities"]:
        if e["card"] == -1:
            if (e["x"], e["y"]) not in TOWER_POS or e["max_hp"] <= 0:
                continue
            objects.append({"nativeObjectId": e["gid"], "owner": e["side"], "cardId": 0, "x": e["x"], "y": e["y"],
                            "hp": max(e["hp"], 0), "maxHp": e["max_hp"]})
        elif e.get("dgid") in known:
            # 正在死亡的单位血量可能是负数，FirstLight 的契约要求非负
            objects.append({"nativeObjectId": e["gid"], "owner": e["side"], "cardId": e["card"],
                            "dataGlobalId": e["dgid"], "x": e["x"], "y": e["y"],
                            "hp": max(0, e["hp"]) if e["max_hp"] > 0 else None,
                            "maxHp": e["max_hp"] if e["max_hp"] > 0 else None})
    for e in frame.get("others", []):
        if e.get("dgid") in known and e["gid"]:
            objects.append({"nativeObjectId": e["gid"], "owner": e["side"], "cardId": e["card"],
                            "dataGlobalId": e["dgid"], "x": e["x"], "y": e["y"], "hp": None, "maxHp": None})
    # 实时对局里同一对象可能出现两次（部队列表和其他对象列表），按对象 ID 去重，否则 BattleEnvV1 报 duplicate identity
    seen, uniq = set(), []
    for o in objects:
        if o["nativeObjectId"] in seen:
            continue
        seen.add(o["nativeObjectId"])
        uniq.append(o)
    objects = uniq
    for i, o in enumerate(objects):
        o["slot"] = i
    # 攻击组件（代理库 "atk":[目标 gid, 阶段, 时间轴, 装填]，2026-11-22 起）：目标必须也在对象列表里才算有效
    owner_of = {o["nativeObjectId"]: o["owner"] for o in objects}
    attack = {}
    for e in frame["entities"]:
        a = e.get("atk")
        if a is None or e["gid"] not in owner_of:
            continue
        tgt, stage = int(a[0]), int(a[1])
        if tgt == 0:
            key, ok = None, True                        # 确认没有目标
        elif tgt in owner_of:
            key, ok = (owner_of[tgt], -2, tgt), True
        else:
            key, ok = None, False                       # 目标不可见 / 不认识：当作未知
        attack[e["gid"]] = (key, ok, stage if 0 <= stage <= 255 else None)
    # 英雄技能控制器（代理库 players[].abil，2026-11-22 起）
    abil = {p["side"]: p.get("abil") for p in frame["players"] if p and p.get("abil") is not None}
    return {"generation": generation, "stateEpoch": 1, "tick": int(frame["tick"]), "ended": bool(frame.get("ended")),
            "finalized": False, "winner": None, "crownsRaw": [0, 0], "players": players, "objects": objects,
            "_attack": attack, "_abil": abil}


def _ability_runtime(raw: dict, owner: int, specs) -> tuple | None:
    """代理库的技能控制器 → RichAbilityRuntimeTelemetry。只收英雄在场（成员 1 个且在对象列表里）、
    技能名在目录里的控制器；配置冷却和最大次数取目录值（16 版技能数据里这两个字段的偏移变了）。
    没有技能数据（旧代理库）返回 None。"""
    rows = (raw.get("_abil") or {}).get(owner)
    if rows is None:
        return None
    present = {int(o["nativeObjectId"]): int(o["owner"]) for o in raw["objects"]}
    out = []
    for r in rows:
        spec = specs.get(r.get("n"))
        st, chg = int(r["st"]), int(r["chg"])
        members = [m for m in r["m"][: int(r["mc"])] if present.get(m) == owner]
        if spec is None or len(members) != 1 or r["adg"] < 1 or r["cdg"] < 1 or not 0 <= st < len(ABILITY_BUTTON_STATE_LABELS):
            continue
        cfg = normalized_ability_cooldown_ms(spec)
        mx = int(spec.charges or 0)
        cd = min(max(0, int(r["cd"])), cfg)
        if (mx > 0 and not 0 <= chg <= mx) or (mx <= 0 and chg != -1):
            continue
        out.append(RichAbilityRuntimeTelemetry(
            controller_slot=int(r["s"]), action_data_global_id=int(r["adg"]), action_data_name=r["n"],
            selected_character_data_global_id=int(r["cdg"]), remaining_cooldown_ms=cd, configured_cooldown_ms=cfg,
            remaining_charges_raw=chg, max_charges=mx, button_state=st,
            button_state_label=ABILITY_BUTTON_STATE_LABELS[st], available=st in ABILITY_QUEUEABLE_BUTTON_STATES,
            champion_entity_keys=((owner, -2, members[0]),)))
    return tuple(out)


def _synth_snapshot(raw: dict, ability_specs=None) -> RichTelemetrySnapshot:
    objs = []
    attack = raw.get("_attack") or {}
    for item in raw["objects"]:
        nid = int(item["nativeObjectId"])
        tkey, tok, stage = attack.get(nid, (None, False, None))
        objs.append(RichObjectTelemetry(
            slot=int(item["slot"]), native_object_id=nid, entity_key=(int(item["owner"]), -2, nid),
            owner=int(item["owner"]), object_index=-1, secondary_index=-1, card_id=int(item["cardId"]),
            data_global_id=item.get("dataGlobalId"), shield_current=None, shield_maximum=None,
            target_entity_key=tkey, target_entity_validated=tok, attack_sequence_stage=stage,
            active_effects=None, invisible_count=None, visibility_state=None, projectile=None,
            entity_resource_runtime=None, periodic_attack_modifier_runtime=None, capture_runtime=None,
            threshold_relocation_runtime=None, phase_runtime=None, tower_troop_runtime=None))
    combat = RichCombatEventEnvelope(
        generation=int(raw["generation"]), state_epoch=1, observation_tick=int(raw["tick"]), capacity=1024,
        hook_set_attested=False, hook_set_installed=False, capability_status="unavailable",
        epoch_first_sequence=0, oldest_retained_sequence=0, next_sequence=0, overflow_count=0,
        sequence_gap_before_oldest=False, rejected_capture_count=0, complete=True, events=())
    players = {}
    for o in (0, 1):
        ab = _ability_runtime(raw, o, ability_specs) if ability_specs is not None else None
        players[o] = RichPlayerRuntimeTelemetry(owner=o, owner_root_validated=ab is not None, owner_entity_key=None,
                                                ability_runtime=ab, evolution_runtime=None)
    return RichTelemetrySnapshot(
        tick=int(raw["tick"]), generation=int(raw["generation"]), state_epoch=1, objects=tuple(objs),
        objects_by_key=MappingProxyType({o.entity_key: o for o in objs}),
        objects_by_native_object_id=MappingProxyType({o.native_object_id: o for o in objs}),
        players=MappingProxyType(players), combat_events=combat, phase_runtime=None, visibility_runtime=None,
        remaining_runtime=None, tower_troop_runtime=None)


class _NullNative:
    """不连接 native。queue_ability_action_at 只是占位：BattleEnvV1 看到它存在才会放开技能动作的掩码；
    真正的技能指令由 FLAgent.finish 生成（离线 ct=189，实战点技能按钮），不经过 env.step。
    ENABLE_ABILITY=False 时去掉这个方法，技能保持屏蔽（旧行为）。"""

    def queue_ability_action_at(self, action, *, execute_in_ticks: int = 1):
        raise RuntimeError("技能指令由 FLAgent.finish 发出，不走 env.step")


# 技能开关（环境变量 CRAI_FL_ABILITY=0 关闭）
ENABLE_ABILITY = os.environ.get("CRAI_FL_ABILITY", "1") != "0"
if not ENABLE_ABILITY:
    del _NullNative.queue_ability_action_at


class EngineBattleEnv(BattleEnvV1):
    def _rich_snapshot_for(self, raw):
        ident = self._rich_identity(raw)
        if self._rich_cache_identity == ident and self._rich_cache is not None:
            return self._rich_cache
        snap = _synth_snapshot(raw, self.ability_specs if ENABLE_ABILITY else None)
        self._prime_rich_cache(raw, snap)
        return snap

    def begin(self, raw: dict, match: MatchConfig):
        """代替 reset：复刻 battle_env.py:858-943 中与 native 无关的部分。"""
        self.match_config = match
        self.episode_config = self._episode_from_match(match, "headless")
        self.episode_id = str(uuid.uuid4())
        self._public_metadata_static = None
        self._episode_start_native_tick = 0
        self._raw = dict(raw)
        self._invalidate_rich_cache()
        self._ordinary_tower_cache.clear()
        self._action_mask_cache.clear()
        self._action_player_runtime_cache.clear()
        self._events.clear()
        self._event_archive.clear()
        self._pending.clear()
        self._pending_abilities.clear()
        self._revealed = {0: set(), 1: set()}
        self._belief_elixir = {p["owner"]: (p["elixirRaw"] / 10000.0,) * 2 for p in raw["players"]}
        self._entity_birth.clear()
        self._previous_entities.clear()
        self._entity_velocity.clear()
        self._last_card = {}          # 每方上一张非镜像出牌（镜像的效果牌）
        self._previous_towers.clear()
        self._terminated = self._truncated = False
        self._decision_deadline_native = {o: int(raw["tick"]) for o in self.possible_agents}
        self._tower_object_keys = self._bind_tower_object_keys(self._raw)
        self._activated_king_tower_ids.clear()
        towers = self._extract_towers(self._raw)
        self._initial_tower_hp = {t.entity_id: t.max_hitpoints for t in towers}
        self._previous_towers = {t.entity_id: t for t in towers}
        ctx = self._rich_context(self._raw)
        self._previous_entities = self._raw_entity_map(self._raw, rich_context=ctx)
        self._prime_combat_event_cursor(ctx.snapshot)
        for eid in self._previous_entities:
            self._entity_birth[eid] = int(self._raw["tick"])

    def ingest(self, raw: dict, plays: list[tuple[int, int, int, int, tuple | None]]):
        """代替 step 的后半段：plays = [(owner, card_id, cost, form_code, position)] 是这一段里双方执行的出牌。"""
        before = self._raw
        before_tick, tick = int(before["tick"]), int(raw["tick"])
        self._raw = dict(raw)
        self._invalidate_rich_cache()
        self._action_mask_cache.clear()
        self._action_player_runtime_cache.clear()
        self._decision_deadline_native = {o: tick for o in self.possible_agents}
        ctx = self._rich_context(raw)
        self._update_beliefs(before_tick, tick)
        for owner, card_id, cost, form_code, pos in plays:
            # 镜像（Mirror）复制自己上一张非镜像牌：效果牌必须是那张牌，否则 FirstLight 的检查会报错，
            # 而且错误事件留在事件列表里，之后每一帧都会报错（2026-11-21 天梯遇到过）。不知道上一张时不发事件。
            effective = card_id
            if card_id == MIRROR_CARD_ID:
                effective = self._last_card.get(owner)
                if effective is None:
                    continue
            else:
                self._last_card[owner] = card_id
            # 己方在途记录（Everlight）：这张牌已经执行，清掉对应的待执行记录
            for rec in list(self._pending):
                if rec.action.owner == owner and rec.card_id == card_id and rec.status == "queued":
                    self._pending.remove(rec)
                    break
            self._revealed[owner].add(card_id)
            low, high = self._belief_elixir[owner]
            self._belief_elixir[owner] = (max(0.0, low - cost), max(0.0, high - cost))
            self._emit_event(tick=tick, event_type="action_executed", owner=owner, card_id=card_id, position=pos,
                             data={"visible_card_id": card_id, "effective_card_id": effective,
                                   "effective_cost": float(cost), "form_code": form_code, "source": "crai_bridge"})
        # 超时没执行的在途记录（被拒、点击失败）清掉，避免一直占着圣水和卡牌特征
        for rec in list(self._pending):
            if tick > rec.expected_native_tick + 40:
                self._pending.remove(rec)
        self._derive_events(before, raw, rich_context=ctx)
        self._terminated = self._terminal(raw, self._extract_towers(raw)).ended


# ---------------------------------------------------------------------------
# 多局批量推理
# ---------------------------------------------------------------------------
def batch_decide(items: list) -> list:
    """items = [(FLAgent, 观测)]（来自 agent.prepare）。按"同一个模型 + 同一种出牌方式（采样/argmax）"分组，
    每组把各局的观测拼成一批、一次前向推理，再逐局解码、记录动作、更新 LSTM 状态。返回与 items 对应的 decision。
    行为与逐局调用 session.decide 相同（同一模型、同样的掩码；批内补齐的部分被掩码屏蔽）。
    需要强制出牌的局（force）很少，单独走原路径。"""
    import torch
    from native_runner.training.v4.decoding import decode_action_sequence_v4
    from native_runner.training.v4.policy_session import PolicyDecisionV4
    from native_runner.training.v4.tensors import RecurrentPolicyStateV4, concatenate_padded_tensor_records

    out = [None] * len(items)
    groups: dict = {}
    for i, (agent, obs) in enumerate(items):
        s = agent.session
        if agent.force:
            out[i] = s.decide(obs, force_act=True)
            continue
        # 不采样的会话在 GPU 上默认走 CUDA Graph（每次把整份局面逐字段拷进固定缓冲区再同步，batch=1 时划算）；
        # 批量路径不用它（剖析：它占了批量推理 70% 的时间）
        s._cuda_graph = None
        groups.setdefault((id(s.model), bool(s.sample), str(s.device)), []).append(i)
    with torch.inference_mode():
        for (_, sample, device), idx in groups.items():
            sessions = [items[i][0].session for i in idx]
            model = sessions[0].model
            hbs = [s.tensorizer.tensorize(items[i][1], validate=False) for s, i in zip(sessions, idx)]
            batch = (hbs[0] if len(hbs) == 1 else concatenate_padded_tensor_records(tuple(hbs))).to_model_input(device)
            state = RecurrentPolicyStateV4(hidden=torch.cat([s.state.hidden for s in sessions]),
                                           cell=torch.cat([s.state.cell for s in sessions]))
            ep = torch.tensor([s._first_decision for s in sessions], dtype=torch.bool, device=device)
            if sample:
                o = model.sample_for_ppo_rollout(batch, state, episode_start=ep, validate=False)
            else:
                o = model.act(batch, state, episode_start=ep, validate=False)
            logp = o.log_prob.reshape(-1).float().cpu().tolist()
            value = o.value.reshape(-1).float().cpu().tolist()
            # 动作一次性搬回 CPU、整批校验一次（以前逐行在 GPU 上校验，每次都强制同步，剖析里占 25%）
            all_acts = o.actions.to("cpu")
            all_acts.validate(sessions[0].tensorizer.config, candidate_count=int(batch.candidates.mask.shape[1]))
            for k, (s, i) in enumerate(zip(sessions, idx)):
                acts = all_acts.index_select(torch.tensor([k]))
                decoded = decode_action_sequence_v4(
                    acts, hbs[k].candidates, row=0, observation=items[i][1], catalog=s.tensorizer.catalog,
                    deck=s.tensorizer.deck, card_costs=s.tensorizer.card_costs,
                    ability_id_by_vocab_id=s.tensorizer.ability_id_by_vocab_id,
                    horizontal_mirror=s.tensorizer.perspective.horizontal_mirror,
                    hand_slot_permutation=s.tensorizer.perspective.hand_slot_permutation,
                    config=s.tensorizer.config, base_latency_ticks=1, base_latency_ms=0.0, validate=False)
                s.tensorizer.record_action(acts, hbs[k], row=0, validate=False)
                if hasattr(s, "record"):                     # PPO 学习方：记录轨迹
                    s.record(hbs[k], acts, logp[k], value[k])
                s.state = RecurrentPolicyStateV4(hidden=o.next_state.hidden[k:k + 1].clone(),
                                                 cell=o.next_state.cell[k:k + 1].clone())
                s._first_decision = False
                out[i] = PolicyDecisionV4(decoded=decoded, inference_ms=0.0)
    return out


# ---------------------------------------------------------------------------
# AI
# ---------------------------------------------------------------------------
class FLAgent:
    """FirstLight V4 模型控制一方。forms0/forms1：{卡牌 ID: el}（1 进化，2 英雄），与引擎卡组一致。"""

    def __init__(self, owner: int, forms: dict[int, dict[int, int]], model="hog2", level: int = 16,
                 sample: bool = False, device: str = "cpu"):
        """model：模型名 / 模型文件路径，或已加载的 LoadedPolicyV4（PPO 训练时学习方与更新共用 GPU 上的同一个模型）。"""
        self.owner = owner
        self.forms = forms
        self.level = level
        self.loaded = load_model(model) if isinstance(model, (str, Path)) else model
        self.sample = sample
        self.device = device
        b = _bundle()
        self.env = EngineBattleEnv(native=_NullNative(), card_catalog=b.native_card_catalog,
                                   semantic_subset_contract=b.semantic_subset_contract, ruleset_id=RULESET_ID,
                                   effect_catalog=RuntimeEffectCatalog.from_catalog(b.native_effect_catalog))
        self.session = None
        self.prev = None          # 上一帧（我们的格式），用来识别出牌
        self.plays = 0
        self.abilities = 0
        self.failed = 0
        # 决策节奏：离线时每 5 tick 正好调用一次；实战读帧时刻不规则，所以按"越过下一个决策点"来触发
        self.next_tz = 0
        self.next_decide = FIRST_POLICY_DECISION_TICK
        self.force = False
        self._subs: dict[int, dict[int, int]] = {}   # 每方：未知卡 → 替身卡

    def _supported(self) -> set[int]:
        if not hasattr(self, "_sup"):
            self._sup = {int(c) for c in self.env.card_specs}
        return self._sup

    def _substitute(self, frame: dict) -> dict:
        """FirstLight 只认识 15 版的 122 张卡。16 版新卡（例如 26000107）会让它在建立对局配置时就报错、整局空转
        （2026-11-21 天梯遇到过）。把卡组里不认识的卡换成同费用、它认识且不在卡组里的卡作为替身（整局固定），
        手牌 pd 里的费用和位置不变，所以出牌和圣水推算仍然正确。"""
        sup = self._supported()
        out = dict(frame)
        players = []
        for p in frame["players"]:
            deck = list(p["deck"])
            if all(c in sup for c in deck):
                players.append(p)
                continue
            subs = self._subs.setdefault(p["side"], {})
            for i, c in enumerate(deck):
                if c in sup:
                    continue
                if c not in subs:
                    cost = cards.cost(c) or 4
                    used = set(deck) | set(subs.values())
                    cand = [s for s in sorted(sup) if s not in used and cards.cost(s) == cost and 26000000 <= s < 29000000]
                    cand = cand or [s for s in sorted(sup) if s not in used and 26000000 <= s < 29000000]
                    subs[c] = cand[0]
                deck[i] = subs[c]
            players.append(dict(p, deck=deck))
        out["players"] = players
        return out

    def _match(self, frame: dict) -> MatchConfig:
        decks, forms = {}, {}
        for p in frame["players"]:
            decks[p["side"]] = tuple(int(c) for c in p["deck"])
            f = self.forms.get(p["side"], {})
            forms[p["side"]] = tuple(int(f.get(c, 0)) for c in p["deck"])
        return MatchConfig(deck0=decks[0], deck1=decks[1], deck0_form_availability=forms[0],
                           deck1_form_availability=forms[1], level_cap=self.level, minimum_card_level=self.level,
                           king_tower_level=self.level)

    def _detect_plays(self, frame: dict) -> list:
        """出牌序号增加的一方：手牌里被换掉的那张就是打出的牌（费用、形态取自上一帧的 pd）。"""
        out = []
        if self.prev is None:
            return out
        prev = {p["side"]: p for p in self.prev["players"]}
        for p in frame["players"]:
            q = prev.get(p["side"])
            if q is None or p["seq"] <= q["seq"]:
                continue
            for i in range(4):
                if q["hand"][i] != p["hand"][i] and 0 <= q["hand"][i] < 8:
                    pd = q["pd"][i]
                    out.append((p["side"], q["deck"][q["hand"][i]], pd >> 28, pd & 0xF, None))
        return out

    def decide_raw(self, frame: dict, account_lo: int, allow: bool = True) -> list[dict]:
        """allow=False：上一次点击还没确认（实战）。只更新环境状态，不调用模型——
        否则模型会把这次被丢弃的动作记进动作历史、登记成在途牌（RoyaleHarness 同样在有未确认动作时不决策）。"""
        obs = self.prepare(frame, account_lo)
        if obs is None or not allow:
            return []
        return self.finish(self.session.decide(obs, force_act=self.force))

    def prepare(self, frame: dict, account_lo: int):
        """决策前半段：更新环境状态。到了决策点返回要交给模型的观测，否则返回 None。
        拆开是为了多局批量推理（crai/env/vec_match.py）：先收集所有局的观测，一次推理，再逐局 finish。"""
        tick = int(frame["tick"])
        self._orig = frame
        self._acct = account_lo
        frame = self._substitute(frame)          # 不认识的新卡换成替身（只影响 FirstLight 看到的卡组）
        raw = to_fl_raw(frame)
        if self.session is None:
            self.env.begin(raw, self._match(frame))
            obs = self.env.observe(owner=self.owner)
            tz = build_episode_tensorizer_v4(self.env.episode_config, actor_owner=self.owner)
            self.session = self._new_session(tz)
            self.session.start_episode(obs, initial_elixir={p["owner"]: p["elixirRaw"] / 10000.0
                                                            for p in raw["players"]})
        else:
            self.env.ingest(raw, self._detect_plays(frame))
            obs = self.env.observe(owner=self.owner)
        self.prev = frame
        if tick < FIRST_POLICY_DECISION_TICK:
            if tick >= self.next_tz:
                self.session.tensorizer.tensorize(obs, validate=False)
                self.next_tz = tick - tick % POLICY_DECISION_TICKS + POLICY_DECISION_TICKS
            return None
        if tick < self.next_decide:
            return None
        self.next_decide = tick + POLICY_DECISION_TICKS
        # 兜底：可出牌后 15 秒（300 tick）还没出过牌就强制出一张（同 FirstLight 评测协议；通用模型面对空场可能一直等）
        self.force = self.plays == 0 and tick >= FIRST_POLICY_DECISION_TICK + 300
        # 实战兜底（stall_guard，只在天梯开）：圣水满了还连续 15 秒没出牌，强制出一张，防止"中途突然不下牌"整局挂机
        if getattr(self, "stall_guard", False):
            me = next((p for p in frame["players"] if p["side"] == self.owner), None)
            if self.plays != getattr(self, "_last_plays", -1):
                self._last_plays, self._last_play_tick = self.plays, tick
            if me is not None and me.get("elixir", 0) >= 9.9 and tick - getattr(self, "_last_play_tick", tick) >= 300:
                self.force = True
                d = self.__dict__.setdefault("diag", {})
                d["stall_forced"] = d.get("stall_forced", 0) + 1
        try:                                        # 排查"中途不出牌"：这一刻掩码里有几张牌能下、不能下的原因
            m = self.env.action_mask(self.owner)
            r = m.reasons or {}
            diag = self.__dict__.setdefault("diag", {})
            legal_play = bool(m.kinds.get(ActionKind.PLAY_CARD.value))
            if not legal_play:
                diag["no_legal_play"] = diag.get("no_legal_play", 0) + 1
            diag["last_mask"] = {"play": legal_play, "eff_elixir": r.get("effective_elixir"),
                                 "reserved": r.get("reserved_elixir"), "pending": r.get("pending_action_count"),
                                 "slots": {s: r.get(s) for s in ("0", "1", "2", "3") if s in r}}
        except Exception as e:  # noqa: BLE001
            self.__dict__.setdefault("diag", {})["mask_error"] = f"{type(e).__name__}: {e}"
        return obs

    def finish(self, decision) -> list[dict]:
        """决策后半段：模型决策 → 原生出牌指令。"""
        orig, account_lo = self._orig, self._acct
        tick = int(orig["tick"])
        me = next(p for p in orig["players"] if p["side"] == self.owner)   # 出牌指令用真实卡组
        cmds = []
        diag = self.__dict__.setdefault("diag", {})                         # 排查"中途不出牌"：模型选了什么、为什么没下出去
        diag["decisions"] = diag.get("decisions", 0) + 1
        if not decision.decoded.actions:
            diag["wait"] = diag.get("wait", 0) + 1
        for a in decision.decoded.actions:
            k = getattr(a.kind, "name", str(a.kind))
            diag[k] = diag.get(k, 0) + 1
            if a.kind == ActionKind.ACTIVATE_ABILITY and ENABLE_ABILITY:
                nid = self._native_of(a.source_entity)
                if not nid:
                    diag["skip_ability_no_native"] = diag.get("skip_ability_no_native", 0) + 1
                if nid:
                    if YES_DELAY:
                        t, t2 = tick, tick + COMMAND_EXECUTION_DELAY_TICKS
                    else:
                        t2 = tick
                        t = t2 - COMMAND_AGE
                    # ct=189：英雄技能，cgid = 场上英雄单位的对象 ID（与重演补技能相同的格式）
                    cmds.append({"ct": 189, "c": {"t": t, "t2": t2, "idHi": 0, "idLo": account_lo, "cgid": int(nid)}})
                    self.abilities += 1
                continue
            if a.kind != ActionKind.PLAY_CARD:
                continue
            slot = int(a.hand_slot)
            d = me["hand"][slot]
            pd = me["pd"][slot]
            if not (0 <= d < 8) or not pd:
                diag["skip_bad_hand"] = diag.get("skip_bad_hand", 0) + 1
                diag["last_bad_hand"] = [list(me["hand"]), [int(x) for x in me["pd"]], slot]
                continue
            gx, gy = a.target_grid
            ox, oy = a.subcell_offset or (0.0, 0.0)
            px = min(17999, max(0, int(gx) * 1000 + 500 + round(float(ox) * 1000)))
            py = min(31999, max(0, int(gy) * 1000 + 500 + round(float(oy) * 1000)))
            card_id = int(me["deck"][d])
            if YES_DELAY:
                # Everlight：下达后 20 tick 生效（与真实游戏一致），触摸延迟档位 0..4 再往后推
                # 下达 tick 不能在将来（引擎解析会拒绝），延迟档位只体现在生效 tick 上
                bin_ = int(a.metadata.get("policy_delay_offset_bin", 0) or 0) if a.metadata else 0
                t = tick
                t2 = tick + bin_ + COMMAND_EXECUTION_DELAY_TICKS
                self._note_pending(a, card_id, slot, d, int(pd), tick, t2)
            else:
                t2 = tick + max(1, int(a.execute_offset_ticks or 1)) - 1
                t = t2 - COMMAND_AGE
            cmds.append({"ct": 153, "c": {"t": t, "t2": t2, "idHi": 0, "idLo": account_lo,
                                          "px": px, "py": py, "sid": -1,
                                          "sel": {"os": card_id, "pd": int(pd)}}})
            self.plays += 1
        return cmds

    def _native_of(self, entity_id) -> int | None:
        """模型观测里的实体 ID（canonical）→ 游戏对象 ID。"""
        if entity_id is None:
            return None
        ctx = self.env._rich_context(self.env._raw)
        for key, cid in ctx.canonical_ids.items():
            if cid == entity_id:
                return int(key[2])
        return None

    def _new_session(self, tz):
        """决策会话；PPO 学习方覆盖这个方法，换成采样并记录轨迹的会话（crai/rl/ppo_fl.py）。"""
        return PolicySessionV4(self.loaded.model, tz, device=self.device, sample=self.sample)

    def _note_pending(self, action, card_id, slot, deck_slot, pd, tick, expected):
        """Everlight 的"在途动作"特征来自环境的待执行记录：登记这次出牌，执行后在 ingest 里清掉。"""
        now = int(time.time() * 1000)
        form = pd & 0xF
        self.env._pending.append(_PendingRecord(
            action=action, card_id=card_id, effective_card_id=card_id, native_effective_card_id=card_id,
            requested_native_tick=tick, expected_native_tick=expected, hand_slot=slot, card_parameter=pd,
            deck_slot=deck_slot, cost=pd >> 28, form_code=form, native_form_code=form, form_name="",
            requested_at_ms=now, generated_at_ms=now, inference_end_ms=now))
