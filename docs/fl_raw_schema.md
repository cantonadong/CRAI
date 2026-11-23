# FirstLight V4 接入：raw 局面格式与 BattleEnvV1 复用方案

源码根：`refs/FirstLight_CR/native_runner/`（下文路径相对于此）。所有结论均已对照代码核实；第 1 节的方案已用 scratchpad 冒烟脚本跑通：用合成的 raw 调用 `observe` → `tensorize` → `PolicySessionV4.decide`（hog26-specialist2，CPU），返回了合法动作（强制出牌得到 `play_card slot1 26000014 (9,9)`）。

## 1. 结论与接入方式

**结论：可以复用，推荐“子类化 BattleEnvV1 + 在 Python 里合成一个空的 rich snapshot”。不推荐直接喂 BattleEnvV1 原版，也不推荐从零手写 ObservationV1。**

- 原版遇到缺失的 rich telemetry 时会直接报错，不会降级：`reset`/`step` 都要经过 `_atomic_observation_pair`（battle_env.py:982-1024），它调用 `native.observe_atomic()`，并用 `bind_rich_telemetry` 严格校验 schema 和 provenance（rich_telemetry_adapter.py:807-850）；`_players` 在拿不到 player runtime 时会 raise（battle_env.py:4447-4449）。这里没有“降级开关”。
- 但 rich 字段在投影层基本都能容忍 None：例如 `project_runtime`（rich_telemetry_adapter.py:3881 起，各字段都是 `if ... is not None`）、`project_player_runtime`（3325/3487，用 `or ()`）；combat ring 在 `hook_set_installed=False` 且事件为空时直接跳过（battle_env.py:3589-3593）；phase/visibility/remaining 为 None 时直接 return（3230-3232）。所以只要喂一个“全空但结构合法”的 `RichTelemetrySnapshot`，后面整套流程都能正常运行。
- 复用的价值：塔绑定、手牌 runtime 契约、Python 落点掩码（含建筑锚点偏移）、spawn/damage/death/tower_damage 事件、速度、圣水阶段和终局判定都由它算好，与训练分布一致。手写这些等于重写约 1500 行。

**具体做法（类 `EngineBattleEnv(BattleEnvV1)`）：**

1. 构造：`EngineBattleEnv(native=NullNative(), card_catalog=bundle.native_card_catalog, semantic_subset_contract=bundle.semantic_subset_contract, effect_catalog=RuntimeEffectCatalog.from_catalog(bundle.native_effect_catalog), ruleset_id=<64位小写 sha256 hex>)`。`bundle = training.v4.factory.production_semantic_bundle()`。
   - 显式传 `ruleset_id` 时 `ruleset_manifest=None`，会跳过 manifest 构建、环境一致性检查和 attestation（battle_env.py:443-462、603-607）。
   - `ruleset_id` 必须是 sha256 hex，否则落点掩码会报错（arena.py:484）。
   - 传入 `native` 后，就不会去构造会连 TCP 的 `NativeClashEnv()`（battle_env.py:401）。`NullNative` 不要定义 `queue_ability_action_at`（技能会被自动 mask，见 battle_env.py:1515/1526），也不要定义 `public_card_play_events_from_combat_ring`（2911）。
2. 覆盖 `_rich_snapshot_for(raw)`：从 raw 合成 `RichTelemetrySnapshot` 后调用 `_prime_rich_cache` 并返回，这样永远不会走到 `_atomic_observation_pair`。合成规则：
   - 每个 raw 对象生成一个 `RichObjectTelemetry(slot, native_object_id, entity_key=(owner,-2,nid), owner, object_index=-1, secondary_index=-1, card_id, data_global_id=<dataGlobalId>, 其余 None/False)`。
   - `players={0,1: RichPlayerRuntimeTelemetry(owner, False, None, ability_runtime=None, evolution_runtime=None 或见 B 类)}`。
   - `combat_events=RichCombatEventEnvelope(..., hook_set_installed=False, complete=True, 各 sequence=0, events=())`。
   - phase/visibility/remaining/tower_troop 一律设为 None。
   - 涉及的数据类定义在 rich_telemetry_adapter.py:461-466、547-563、753-799。
3. **不要调用 `reset`/`step`**（它们会连 native）。用自己的 `begin(raw, episode)` 代替 reset，它复刻 battle_env.py:858-943 中与 native 无关的部分：
   - 设置 `match_config`：可用 `MatchConfig(...)`，也可从 `EpisodeConfigV1` 按 728-751 还原，`_episode_form_mask` 依赖它。
   - 设置 `episode_config`、`episode_id=uuid4`、`_raw`，并执行 `_invalidate_rich_cache()`。
   - `_belief_elixir`、`_tower_object_keys=_bind_tower_object_keys(raw)`。
   - `_initial_tower_hp`/`_previous_towers`、`_previous_entities=_raw_entity_map(...)`、`_prime_combat_event_cursor(snapshot)`、`_entity_birth`。
   - 调用 `semantic_subset_contract.assert_deck(...)` 自检卡组和形态（815-822）。
4. 每个决策点调用一次 `ingest(raw)`，复刻 2743-2777：
   - 记 `before=self._raw`，再设新 `_raw`、执行 `_invalidate_rich_cache()`，并令 `_decision_deadline_native={0:tick,1:tick}`。如果不这样做，`action_mask` 只会返回 WAIT（2008）。
   - 调用 `_update_beliefs(before_tick, tick)`。
   - 为双方的新出牌生成 `action_executed` 事件，见第 2 节 B 类。
   - 调用 `_derive_events(before, raw, rich_context=self._rich_context(raw))`，它会生成速度、spawn、damage、death、tower 事件（3894-4003）。
   - 调用 `_terminal(...)`。
5. 取观测：`obs = env.observe(owner=actor)`，然后交给 `PolicySessionV4(model, build_episode_tensorizer_v4(episode, actor_owner=actor), device=...)`。不要用 `build_policy_session_v4`，它要求传入 env，而且会校验 `decision_hz==4`（policy_session.py:337-357）。

## 2. raw 字段清单

坐标、单位和 tick 都是引擎原生值，详见第 4 节。

**A：必需（缺了会报错）**

| JSON 路径 | 类型 | 含义 / 约束 | 例 |
|---|---|---|---|
| `tick` | int | 战斗 tick（GameStateManager），开局为 0 | 90 |
| `generation` | int | 对局标识，单局内保持不变（缓存键，battle_env.py:972-980） | 1 |
| `players[i].owner` | 0/1 | 两个玩家都必须提供 | 0 |
| `players[i].elixirRaw` | int | 圣水×10000 | 70000 |
| `players[i].hand[]` | list | 只列出非空槽 | |
| `  .handIndex` | int | 手牌位 0..3，也是出牌指令里的 hand_index | 1 |
| `  .cardId` | int | 卡组卡 ID，不是形态 ID | 26000014 |
| `  .cost` | int | 等于 pd>>28；None 会导致该槽被 mask | 4 |
| `  .cardParameter` | uint32 | pd，格式见第 3 节；None 会导致该槽被 mask | |
| `  .deckSlot` | int | 等于 ((pd>>22)&0x3f)-1 | 1 |
| `players[i].cycle[].cardId` | list | 不在手里的牌，按队列顺序排列。**hand+cycle 必须恰好覆盖 8 张卡组牌**（tracking.py:645-658） | |
| `players[i].deck[].cardId` | list(8) | 按原生卡组槽顺序 | |
| `objects[]` | list | 塔 + 场上对象 | |
| `  .slot` | int | 0..n-1 连续编号，作为 rich snapshot 的索引 | |
| `  .nativeObjectId` | int>0 | 实体 +0x08，同一 tick 内唯一 | |
| `  .owner` `.x` `.y` | int | +0x78 / +0x7c / +0x80 | |
| `  .cardId` | int | +0xac；≤0 时视为 effect | 26000021 |
| `  .hp` `.maxHp` | int/null | 塔必须有 maxHp>0 | |
| `  .dataGlobalId`（合成 rich 用） | int | +0x48→LogicData+0x10。**非塔实体缺失时会报错**：tensorizer 以 `reject_unknown_public_semantics=True` 运行，archetype 无法识别时会 raise（factory.py:204，tensorizer.py:1313-1347） | 34000021 |

- **塔**必须在 `TOWER_LAYOUT` 指定的精确坐标上，且每个位置恰好一个对象（battle_env.py:138-145、1044-1065）：owner0 王塔 (9000,3000)，公主塔 (3500,6500)/(14500,6500)；owner1 王塔 (9000,29000)，公主塔 (3500,25500)/(14500,25500)。
- 非塔对象中，如果 dataGlobalId 不在 `entity_archetype_catalog` 里（例如 player-root 或其他内部对象），应在 bridge 里过滤掉，否则 tensorizer 会 raise。已知 ID 示例：HogRider 34000021、Skeleton 34000008、Musketeer 34000014、MusketeerHero 130283371、Cannon 35000002、Cannon_EV1 140000000、IceGolemite 34000036、IceSpirits 34000031；投射物和区域效果用各自的 global id。字典在 `data/competitive/model_catalogs.json.gz` 的 `catalogs.entity_archetype_catalog`。**16.635 的 ID 需要逐个核对**，hash 形式的新行 ID 可能变化。

**B：影响模型输入，可填默认值**

- **出牌事件（强烈建议补上）**：对每次出牌调用 `_emit_event(event_type="action_executed", owner, card_id, position, data={...})`，字段照抄 battle_env.py:3032-3051，至少包含 `effective_cost`、`form_code`、`visible_card_id`、`effective_card_id`。同时执行 `_revealed[owner].add(card_id)`，并扣减 `_belief_elixir`。
  - 对手明牌和对手圣水区间完全靠这个事件推算（tracking.py:89、545-551）。`card_id` 必须在该方的 episode 卡组里（tracking.py:333-335）。
  - 我方出牌时需要先放入 `_pending`（`_PendingRecord`，battle_env.py:217-235），以便预留圣水。执行后清除，并同样发出 `action_executed`。
- **进化进度**：在 `RichPlayerRuntimeTelemetry.evolution_runtime` 中填入 `RichEvolutionRuntimeTelemetry(deck_slot, card_id, base_spell_global_id, evolvable=True, evolution_form_global_id=None, progress, cycle_required=2, cycle_remaining, ready)`（rich_telemetry_adapter.py:448-457、3487-3536）。不填时，卡牌特征里的进化项为 0。
- `crownsRaw:[a,b]`、`ended`/`finalized`、`winner`（battle_env.py:1410-1469）：缺失时从塔血推算。
- `stateEpoch`：可为 None。

**C：可省略**：`accountId`（只在出牌时用到）、`targetX/Y`、`objectIndex/secondaryIndex`、`commandCardId`（只有 Mirror 需要）、`formCode`（缺失时从 pd&0xF 推导，battle_env.py:1696-1705）、`count`、`queuedCommands`、`truncated`。所有 rich 字段（攻击阶段、目标、护盾、buff、可见性、投射物等）也都可以省略，代价是对应特征为 0，属于分布偏移，但流程不会报错。

**技能**：只要 `ability_runtime=None`，`activate_ability` 永远被 mask，Hero 火枪的技能不会被使用。要启用技能，需要：
- 填入 `RichAbilityRuntimeTelemetry`（rich_telemetry_adapter.py:432-444）：数据来自 player+0x3a0/0x3a8 的 champion controller，`champion_entity_keys` 指向场上火枪实体，`action_data_name` 设为 `Musketeer_hero_Ability`；
- 在 `NullNative` 上实现 `queue_ability_action_at`。

## 3. FirstLight probe 的取数方式（15.535 偏移）

- 玩家（cr_replay_probe.cpp:2721-2737）：`world=manager+0xa8`；`player=world+0xe0+owner*8`（也是王塔 root）；`identity=world+0x30+o*8`，accountId 高/低位在 +0/+4；`deck=world+0x88+o*8`，卡组槽向量在 deck+0x20；`elixir=player+0x2f8`；手牌向量 `player+0x220`（元素是卡组槽号，-1 表示空洞）；cycle 向量 `player+0x230`（cycle[0] 是 nextCard）。
- 卡牌（2751-2800）：`card_data=slot+0x10`，`cardId=card_data+0x40`。**pd 来自游戏函数 `CardSelection::build(battle_deck_slot, player, 0, &out)`**（`g_card_selection_build`），取 `out.packed`。位定义（cr_native_env.py:183-212）：`bits0-3 = form`（0 Basic / 1 Evo / 2 Hero），`bits22-27 = deckSlot+1`，`bits28-31 = cost`，其余位保留不解释。Mirror 的情况下，form==0 且 secondary_data 存在时，`commandCardId=secondary+0x40`。
  - 进化：只有进化就绪时 pd 低 4 位才是 1，此时 `form_code` 为 1。
  - Hero 火枪：`cardId` 仍是 26000014，pd 低 4 位为 2。冒烟测试已验证 form 1 和 form 2 都能通过契约。
- 对象（4401-4630）：对象向量在 `player0+0x10 → +0x08`，capacity/count 在 +0x10/+0x14。`nativeObjectId=+0x08`（非 0，且向量内唯一），`owner=+0x78`，`x/y=+0x7c/+0x80`，`targetX/Y=+0x84/+0x88`，`objectIndex/secondary=+0x94/+0x98`，`cardId=+0xac`。`dataGlobalId=(+0x48)→+0x10`（4683）。hp：组件数组在 `+0x18`，组件 `[2]` 满足 `comp+0x08==object` 时，`comp+0x10/+0x14` 分别是 hp/maxHp（4315-4337）。
- 终局（2312-2330）：`finalized=world+0x1e0`，`worldResult=world+0x1b8`（值为 0/1 时即 winner）。
- 进化进度在 `player+0x2e8`，门槛来自 EvoForm+0x284（4841-4847）。
- **塔识别**：只按 `TOWER_LAYOUT` 的坐标、owner 和 maxHp>0 绑定，不看 vtable。之后通过 nativeObjectId 跟踪，塔死亡后对象消失即视为 inactive（1044-1102、1240-1401）。
- **落点掩码由 Python 计算，不来自 probe，也不读 LocationTileMap**：
  - 地形是 arena.py:180-207 里硬编码的 18×32 粗格（在 v15 上逐格探测过）；
  - 再叠加卡牌规则、已摧毁公主塔对应的口袋、塔和建筑的占位矩形，算法在 `card_placement_mask`（arena.py:705）；
  - 建筑另外返回 `model_subcell_offset`。
  - 如果 16.635 的地形或口袋规则有变化，需要修改这里。

## 4. 坐标系

raw 里的 x/y 直接使用引擎原生坐标（0–18000 × 0–32000，1 格 = 1000）。**owner0 在 y 小的一侧**（王塔 y=3000，为 True Red）。`ObservationV1` 本身**不翻转**，翻转发生在 tensorizer 内部：actor=1 时 `flip_x=flip_y=True`（perspective.py:77-82，tensorizer.py:471-481）。解码出的 `ActionV1` 已经转回原生坐标系（decoding.py:548）。

## 5. EpisodeConfigV1（野猪 2.6）

训练时的写法见 ppo_runtime.py:70-96。卡牌等级只训练过 11 和 16（matchmaking.py:25），要求统一等级，王塔等级相同。form 位：bit0=Evo，bit1=Hero（match_factory.py:68-82，factory.py:90-100）。

```python
DECK  = (26000021, 26000014, 27000000, 28000000, 28000011, 26000010, 26000038, 26000030)
#        hog       musk-hero cannon-ev1 fireball  log       skel-ev1  ice-golem ice-spirit
FORMS = (0,        2,        1,         0,        0,        1,        0,        0)
episode = EpisodeConfigV1(ruleset_id=RID, deck0=DECK, deck1=OPP_DECK, seed=1,
    decision_hz=4.0, event_driven_decisions=False,
    tags={"deck0_form_availability": FORMS, "deck1_form_availability": OPP_FORMS,
          "tower_troop0_id": 159000000, "tower_troop1_id": 159000000,
          "level_cap": 11, "minimum_card_level": 11, "king_tower_level": 11,
          "end_tick": 7200})
```

- deck/forms 必须和引擎里的卡组**按槽一一对应**。对手卡组也必须真实，tracker 会校验对手出的牌是否在其卡组内。
- 以上卡牌 ID 均已在 catalog 中核实。semantic subset 允许的形态为：火枪 3、加农 1、骷髅 1、冰人 2、冰精灵 1、野猪/火球/滚木 0。

## 6. 动作 → 原生指令

`decision.decoded.actions` 是原生坐标系下的 `ActionV1`，最多 2 个；WAIT 时没有指令。

- **PLAY_CARD**：
  - `hand_slot` 就是原生 `handIndex`；
  - 落点：`(gx,gy)=target_grid`，`px=gx*1000+500+round(subcell_offset[0]*1000)`，`py` 同理，clamp 到 [0,17999]/[0,31999]（battle_env.py:2321-2327，arena.py:158-166）。建筑带有 `subcell_offset`，owner1 时符号已经反转；
  - 执行时刻：`execute_tick = obs_tick + execute_offset_ticks`，范围 1..5，等于 1 加延迟档位（0..200ms 对应 0..4 tick，decoding.py:330-341）；
  - 指令（cr_native_env.py:2883-2975）：`t2 = execute_tick - 1`，`t = t2 - 20`，`{"ct":86→我方153,"c":{"t","t2","idHi","idLo","px","py","sid":-1,"sel":{"os":commandCardId(通常=cardId),"pd":cardParameter}}}`。pd 直接使用**观测时**手牌的 pd，不需要重新编码；
  - `execute_tick` 至少为 91（FIRST_PLAYABLE_TICK+1）。
- **技能**：`{"ct":2→189,"c":{"t":T,"t2":T+1,"idHi","idLo","cgid"}}`（cr_replay_probe.cpp:5893-5905）。`cgid` 是**该 Hero/Champion 实体的 +0x08**，即 nativeObjectId（5794）。要求 controller 处于可排队状态、冷却为 0、charges≠0（5708-5780）。

## 7. 时间

- `FIRST_POLICY_DECISION_TICK=90`，`POLICY_DECISION_TICKS=5`（training/v4/expert.py:26-27）；`FIRST_PLAYABLE_TICK=90`，`LIVE_COMMAND_AGE_TICKS=20`（cr_native_env.py:36-37）。
- 流程参照 offline_duel.py:63-126：
  1. 开局后用第一份观测调用 `session.start_episode(obs, initial_elixir={0:e0,1:e1})`，两方都传精确圣水；
  2. 当 tick<90 时，**只调用 `session.tensorizer.tensorize(obs, validate=False)`** 来推进 tracker，不做决策；
  3. tick≥90 后，每 5 tick 调用一次 `decide`。训练中 headless 模式先 warmup 到 90，之后决策点为 90, 95, 100…。必须每 5 tick 都 decide，WAIT 也要算，因为 LSTM 状态和 previous_action 依赖逐步推进（policy_session.py:331-333）。
- `_canonical_tick(t)=t-_episode_start_native_tick`，其中 `_episode_start_native_tick` 固定为 0（battle_env.py:864、1471-1472），所以 obs.tick 就等于原生 tick。
- 圣水阶段和倍率由 `mode_timeline.phase(native_tick)` 给出；`remaining_ms=(6000-tick)*50`（battle_env.py:4625、4641）。我方引擎的 tick 原点必须与 v15 一致，即开局 tick=0、可出牌 tick=90、常规时间加加时共 6000 tick，需要实测核对。
