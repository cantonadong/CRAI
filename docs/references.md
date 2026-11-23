# 参考资料要点

参考仓库已 clone 到 `refs/`（Jason-XII，均无 License，只作参考，不直接复制代码）。

## cr-memory-reader（内存读取，最重要）
- `refs/cr-memory-reader/My Own Findings.md`：ARM64 版 Null's（13.300.6，包名 `nullsroyale.rel.free`）的逆向过程。
  - 网络：消息 18411 是客户端下牌；26349 是开局大包；23877 是结算（里面有双方卡组）。对局中没有连续的状态同步包，所以**只能读本地内存**。
  - 关键结构：
    - `holder+0xa8 → ui_state`：`ui+0x1e0` 是我方圣水（int），`ui+0x220` 是战斗时钟（float）。
    - `holder+0x290 → model`：`model+0x220` 是手牌 deck index[4]，`model+0x210` 是下一张牌队列（数组指针在 `+0x20`，数量在 `+0x2c`）。
    - 卡牌数据：`card_data+0x40` 是卡牌 ID（如 26000018）。
    - 实体：`+0x30` kind，`+0x78` side，`+0x7c/+0x80` x/y（0–18000 / 0–32000），`+0xac` card id，`+0x120` 等级索引。HP 的取法是先读 `vtable+0x120` 找到 HP 访问函数拿到 component，再读 `component+0x10`。
- `refs/cr-memory-reader/x86_64/内存读取技术指南.md`：**x86_64 版的只读读取方案，直接适用于 LDPlayer**。
  - 原理：root 后读 `/proc/<pid>/mem`，不注入（Frida 附加会导致客户端重启）。
  - 全局指针链：`libg+0x1a85838 → battle_manager → +0x20 context → +0x90 battle → +0xa8 roster → +0xe0 双方 runtime player`。
  - player 字段：`+0x2f8` 原始圣水（÷10000），`+0x258/+0x318` 动作快照和动作序号。
  - 定位 holder 的方法：在 Scudo 堆的 size class 1/2/15/16 区域里扫描，再用多个关联字段（canary）交叉校验。
  - 以上偏移都锁定在特定版本（versionCode 150535022，以及该版本 libg.so 的 SHA-256）。**版本不同必须重新标定**，标定流程见该文第 11 节。
  - 源码：`x86_64/tools/native-stream/cr_arm_entity_stream.c`、`x86_64/deploy/proc_memory.py`。
- `Rocca-reference/`：Giovanni Rocca 写的 Supercell 逆向系列文章（加密、反调试）。

## daybreak-cr（AI 训练与实战）
- `docs/GAME_INTEGRATION.md`：下牌发出后 21 tick 生效；服务器 10 tick 一回合；对手的下牌提前 2–21 tick 就能在队列里看到。
- `cr/observation.py`、`cr/policy.py`：观测设计和策略网络（卡牌嵌入 + 棋盘编码 + 落点格子头）。
- `assets/`：卡牌费用、落点掩码、卡牌词表。
- 私有、拿不到的部分：clapha-engine 引擎、RoyaleAPI 回放数据集、内存读取器。

## clash-royale-simulator
- 纯 Python 战斗模拟器，只有 47 张卡。后期可作离线训练或调试的备选。

## FirstLight_CR（最接近我们路线的完整项目）
- 来源：https://github.com/Jaasssoooonnnnn/FirstLight_CR，已 clone 到 `refs/FirstLight_CR`。**Apache-2.0**，可以复用代码，但要保留版权声明和 NOTICE。
- 作者用其中的 2.6 野猪专精模型打上了名人堂。
- **整体思路**：人类回放 → 在游戏引擎里重建对局 → 得到逐帧观测和动作 → 模仿学习（IL）→ PPO 自博弈 → 评测。我们的"回放注入重演"是同一个思路，只是实现方式不同。
- **引擎用法（和我们的差别）**：
  - 他们给 Null's 的 APK 打补丁，注入 ARM64 的 native probe（`native_runner/probe/cr_replay_probe.cpp`），直接调用引擎函数开局、下牌、放技能。可以不渲染、一个进程同时跑多局（常驻 slot），并提供 Python 的 `reset`/`step` 接口（`battle_env.py`）。
  - 我们是借游戏内回放播放器、改写指令内存，受底子回放容量（57 条）限制。他们的方式没有这个限制。
  - 版本锁定为 Null's 15.535.13 arm64（MuMu，Android 12）。我们用的是 16.635.1，偏移不能直接用。
- **直接能解决我们当前限制的内容**（`native_runner/royaleapi_replay.py`）：
  - **tick 含义**：RoyaleAPI 的指令 tick N，对应引擎里第 N+1 tick 才能观测到（`ROYALAPI_COMMAND_TO_NATIVE_OBSERVABLE_TICKS = 1`）。回放时间按 20 Hz 计。
  - **英雄技能**：RoyaleAPI 的技能事件不带英雄名，他们从该方卡组里的英雄卡（`-hero` 后缀）和冠军卡推断技能来源；冠军卡号与技能的对应表在 `normal_form_evidence.py`（`NATIVE_CHAMPION_ABILITY_BINDINGS`、`NATIVE_HERO_FORM_ABILITY_BINDINGS`）。
  - **进化 / 英雄形态**：卡名后缀 `-ev1`、`-hero` 表示形态，卡组带 `form_availability` 掩码。
- **数据集**：Hugging Face `VanguardX101/IL_Replay`，25 万局回放、1784 万条动作，Parquet 格式（schema `royaleapi-battle-replay.v1`，含卡组、形态、塔兵、20 Hz 动作时间、坐标）。比我们的 72 局大几个数量级。
- **模型**（`training/v4/model.py`、`tensorizer.py`）：循环 actor-critic，输入卡牌嵌入、实体关系、空间特征；每 5 tick 决策一次。
- **训练经验**（`docs/training_history.md`）：
  - 只看对固定 IL 对手的胜率会被误导：80% 胜率里绝大多数是对手几乎不出牌的局。要同时看回放和对手活跃度。
  - 野猪专精：从通用模型初始化，加入真人野猪回放示范、PPO，再去掉野猪专用奖励继续 PPO，最终版胜率更高但打法更被动。
  - 圣水溢出惩罚有用；必须保证被拒绝的下牌动作为 0。
- 随附模型 checkpoint（各约 48 MB）在 `checkpoints/`，可 CPU 推理。
- 主分支的模型下牌立即生效；`yes-delay` 分支的 Everlight 模型和真实游戏一样有 20 tick 延迟，**上真实天梯应该用这个分支的模型**。
- 他们的 probe 通过 `kReplayJsonSetterOffset` 把回放 JSON 直接交给游戏的回放系统，所以没有指令容量限制。

### 迁移评估（2026-11-21）
| 路线 | 要做的事 | 难点 |
| --- | --- | --- |
| A. 照搬：降级到 15.535.13 + MuMu | 找原版 15.535.13 APK，装 MuMu（Android 12，root，ARM64），按 `docs/getting_started.md` 构建离线 APK | 我们手上只有 16.635.1（`apk/pulled/base.apk`）。旧 APK 还要能从服务器下到 15.535.86 资源，服务器可能已强制升级。还要装 JDK 17（本机没有 java）。 |
| B. 把 probe 移植到 16.635.1 | 重新定位 probe 里约 200 个 libg 函数偏移和结构布局 | 工作量最大，相当于重做他们的逆向。 |
| C. 只用他们的模型，接我们的读取器和点击下牌打真天梯 | 用 `cr_reader` 的数据拼出 FAIR `ObservationV1`，调用 `serve_policy`，把返回的 `ActionV1` 转成点击 | `ObservationV1` 字段很多（实体攻击阶段、效果、技能状态等），需要先查 tensorizer 实际用到哪些字段；读取器可能要补字段。不需要换游戏版本。 |
| D. 只用他们的数据集 | 下载 HF 的 IL_Replay，用我们的注入重演生成训练数据 | 仍受底子容量限制，重演速度也慢（实时播放）。 |

本机条件：RTX 5070 Ti（12 GB）、NDK 27.2、build-tools 34/36、Python 3.13（他们要求 3.12），没有 JDK、没有 MuMu。

结论：**先试 A**（只要拿到 15.535.13 APK 并能更新到 15.535.86，就能直接得到离线引擎、25 万局数据和名人堂级模型）；A 走不通就做 C。

## 新参考仓库评估（2026-11-22，clone 在 `refs/new2611/`）
FirstLight_CR 在 GitLab 上的版本（2026-09-06）比 `refs/FirstLight_CR`（GitHub 版，2026-10-31）旧，没有新内容。

### RoyaleHarness / Clashaiaa（Apache-2.0，可复用，需保留 NOTICE）
- 两者都是 FirstLight V4 模型在线实战的桥接层：探针读状态 → 模型 → ADB 点击 → 确认执行结果。它们锁定的 libg SHA 等于 15.535.13，说明当时 15.535.13 还能联网打天梯；现在是否还能连上未验证。
- 我们实战观测和他们的差距：
  - `fl_agent.py:153-177` 把攻击目标、投射物、效果、ability/evolution runtime 都设为 None，这些都是 tensorizer 实际用到的输入（`tensorizer.py:1100-1123`，以及 TARGETS 关系边）；
  - 没有 causal_group（`grouping.py:49-51` 会退化成每个单位单独一组）；
  - 在途的牌模型看不见：他们把已下未落地的牌作为带倒计时的虚拟单位加进画面（`feature_adapter.py:718-787`）。
- 执行逻辑（`agent/execution.py`）：
  - 预扣圣水、锁槽位并加进掩码；
  - 点击前用最新帧复核；
  - 0.35 秒内用手牌变化或圣水下降确认，否则记为 UNKNOWN，该槽冷却 1 秒；
  - 敌我单位按延迟往前外推（`feature_adapter.py:620-716`）；
  - 法术禁打国王塔，防守走错路时否决。
- 英雄技能：
  - 探针读取 `player+0x3a0`（15 版偏移）的两个技能 controller；
  - 可释放条件：冷却 0、次数 1；点击按钮后按次数从 1 变 0 确认；
  - 还要给 `_NullNative` 加 `queue_ability_action_at` 桩函数，否则技能一直被屏蔽。
- 探针在游戏线程里 hook `BattleController::fullUpdate`，每个 tick 推送一次完整快照（`nulls_probe.cpp`）。基础结构偏移和 16 版一致，可以照着移植。
- Clashaiaa 新增的"局面预测"镜像，实机上实际是停用状态（缺 rich 字段、卡组格式不匹配），不值得搬。

### Clash-Royale-Battle-Engine（Jason-XII，无 LICENSE，只借鉴思路）
- 实战采样：选牌和选格子用 T 0.05（相当于 argmax），出牌时机保持 T=1 采样；时机温度偏离 1 会明显变差。我们现在三个头全是 argmax，需要 A/B 测。
- 时机头是影响最大的部分（TRAINING.md §10）。
- 游戏机制（GAME_INTEGRATION.md）：
  - issue 后 21 tick 执行，第 22 tick 单位上场；他们实战固定在"决策后 26 tick 落地"，和训练对齐；
  - 补牌前点那个槽位完全无效；
  - tick 94 前的点击无效。
- PPO：
  - KL 锚定当前冠军而不是 IL，系数 ≤0.01；
  - 专精版加入 deck_mix（focus/hard 各 0.3）和 bc_coef 0.05–0.2（用人类赢家对局做行为克隆）；
  - 门控晋升：53%；连续两次 <45% 就回滚；
  - 天梯输局集中在重坦和空军卡组。
- 格子头要用 fp32（bf16 下会大量出现最高分并列）。

### clashroyal-replay（无 LICENSE，只借鉴思路）
- 在腾讯 38.x 官方客户端里用 LLDB 加 dlopen 调用原生引擎重演 RoyaleAPI 回放。
- RoyaleAPI 对战行 HTML 里卡牌的 `data-card-key` 带 `-ev1`/`-hero`，也就是有形态信息，还有等级、塔兵、六塔终局血量、溢出圣水。
- 随机种子 `rndSeed` 只影响发牌、猎人散射、墓园落点这类随机，只在打出猎人或骷髅帝王时才值得搜。
- 英雄技能：枚举"已充能的候选英雄"，做分支定界，再用溢出圣水筛选。匹配指标是六塔血差、终局时间、溢出圣水。
- 它的 browser-extension 在列表页批量取 `/data/replay`，同时保存对战行。

## RoyaleAPI 回放
- 接口：`GET https://royaleapi.com/data/replay?tag=&team_tags=&opponent_tags=&team_crowns=&opponent_crowns=&referrer_path=`，参数来自战斗记录页 `.replay_button` 的 `data-*` 属性（见 `replay_greakmonkey_script.js`）。
- 解析器：`crai/data/replay.py`，测试样例在 `tests/fixtures/replay1.json`。
- 回放的坐标单位和内存里的实体坐标一致，都是 1/1000 格（18×32 格）。所以回放的落点可以直接对应到实战坐标。
