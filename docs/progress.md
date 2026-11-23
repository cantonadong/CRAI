# 进度与待办（更新：2026-11-21 凌晨）

## 已完成
- **读内存**：`crai/reader/cr_reader.c`，约 29 帧/秒。
  - 能读双方圣水、手牌、队列、卡组、出牌序号、最近出牌，以及实体位置和血量（实体来自游戏内部列表）。
  - 偏移见 `configs/nulls_16.635.1.json`，细节见 `docs/re_notes.md`。
- **下牌**：默认点击方式（`crai/client/control.py`），目标格命中约 79%，其余偏 1 格。标定文件 `configs/screen_1080x1920.json`。
- **自动化**：
  - `scripts/start_training.py`：训练营或天梯开局，会先识别界面并回到主界面。
  - `scripts/run_bot.py N training|ladder rules|il`：bot 连续对战。
  - `scripts/record_games.py`：被动录制对局和回放。
- **规则 bot / 模仿学习 bot**：训练营能赢；天梯对 6300+ 奖杯的真人基本全输。主要原因是回放数据里没有单位实时状态。
- **回放注入重演（关键突破）**：`scripts/inject_replay.py <RoyaleAPI回放>`
  - 打开对战日志第 1 局回放当底子，开播 2.6 秒内一次写完：双方卡组（16 级、关闭进化）、反推的开局手牌、全部出牌指令。
  - 野猪 2.6 一方固定放在下方，必要时坐标旋转 180°。
  - 游戏引擎完整重演，读取器录下每一帧，存到 `data/resim/<回放名>.jsonl`。
  - 结局核对：replay0013 两次重演都是 3:0，与原局一致；replay0001 重演 0:1，原局 0:3，不一致（对手用了弓箭女皇，技能指令缺失）。
- 批量脚本：`scripts/batch_resim.py [容量]`，结果追加到 `data/resim/_summary.jsonl`。

## 当前限制
1. **底子容量 57 条指令**：72 局里只有 15 局能装下。指令对象由游戏分配、执行完就释放，没法新建，所以要更长的底子回放。
2. **英雄技能**：RoyaleAPI 有技能释放的 tick（`_invalid` + `data-ability=1`），但没有英雄名和坐标；技能指令的格式还没解析。
3. **进化卡**：RoyaleAPI 回放没有进化标记，注入时全部按非进化处理，可能和原局有偏差。
4. minion-giant 这张卡在当前客户端里不存在，相关出牌会被跳过。

## 路线调整（2026-11-21）
发现参考项目 FirstLight_CR（见 `docs/references.md`），它已经实现了引擎内重演、IL+PPO 训练，并公开了数据集和模型。用户决定**先评估迁移成本，暂不打天梯**。评估表见 `docs/references.md` 的"迁移评估"。
下一步：
1. 问用户能否弄到 Null's 15.535.13（arm64）原版 APK；能的话装 MuMu（Android 12，root，ARM64）和 JDK 17，试路线 A。
2. 拿不到就走路线 C：查 `refs/FirstLight_CR/native_runner/training/v4/tensorizer.py` 实际用到的 ObservationV1 字段，对照我们读取器能读到的字段。

### 用户定的方向（2026-11-21）
保持 16.635.1，自己改 APK：移植 FirstLight 的离线引擎 probe（只要 2.6 卡组需要的部分），复用他们的模型和回放数据；模型打天梯，同时继续训练。
- 改过的 APK 装在**另一个 LDPlayer 实例**里离线用（签名变了，不能覆盖天梯账号那个）；天梯仍用原版 APK + 读取器 + 点击下牌。

### 函数地址移植（进行中）
- 旧版 APK：`old_null_apk/15.535.13.apk`；其 libg 已解出到 `apk/libs_15535/libg.so`，SHA-256 与 FirstLight 记录一致。
- Ghidra 工程 `re/ghidra/nulls_15_535_13`（新版是 `nulls_16_635_1`）。特征导出脚本 `re/ghidra_scripts/ExportFuncFeatures.java` → `re/feat_*.jsonl`。
- 匹配：`python scripts/fl_match.py re/feat_15535.jsonl re/feat_16635.jsonl`，旧 50471 个函数对上 38601 个（76%）。结果 `re/fl_funcmap.json`。
- probe 偏移映射 `re/fl_probe_offsets.tsv`：147 个里，函数 67 个已对上、9 个没对上；71 个是 vtable、全局变量、函数内调用点，要另行映射。
- 已知可疑：GameStateStep（相似度 0.78，与 Tick 的间距和旧版差很多）、JsonMemberFind（位移方向和周围函数相反），要人工核对。

- 第二轮（`scripts/fl_map_addrs.py`、`scripts/fl_refine.py`，加调用点投票、虚表槽位、PLT/GOT 符号、字符串、反编译对照 `re/dec_old.c`/`re/dec_new.c`）：
  - **离线引擎核心 45 个地址：44 个已定位**，结果在 `configs/fl_engine_16.635.1.json`（ok 37 个，check 6 个待反编译核对）。
  - 16 版把"加载回放"的 JSON 解析挪进新辅助函数 `0x12d90ec`，GameStateManager 有字段偏移变化 → probe 的钩子逻辑要按新结构改写，不能只换地址。
  - SHA256 自检的调用方式变了（疑似新增混淆），只有换 MuMu 才需要处理。
  - 遥测类地址（战斗事件、冠军、塔兵等，`re/fl_addr_map.tsv`）先不管，2.6 卡组用不到大部分。
- 已确认用户卡组形态和 FirstLight 野猪专精模型训练卡组完全一致（进化加农炮、进化骷髅兵、英雄火枪手）。

### 离线引擎已跑通（2026-11-21）
- 在天梯实例里运行，不改 APK。详情见 `docs/re_notes.md` 的"离线引擎"。
- 工具：
  - `scripts/royaleapi_to_native.py <回放>`：RoyaleAPI 回放 → 原生 JSON，输出到 `data/native_replays/`；
  - `scripts/headless_resim.py <原生json>`：重演，输出到 `data/resim_headless/`；
  - `scripts/check_resim.py`：逐条核对指令是否被执行。
- **没有指令容量限制；一局约 0.2 秒。**
- replay0076（93 条指令）在野猪方标上 2.6 形态后，重演 1:0，与原局一致。
- 限制：RoyaleAPI 回放没有形态信息。对手的进化卡只能按普通处理，野猪方按惯用形态推测。FirstLight 的 HF 数据集带形态，可以解决这个问题。
- 代理库**目前仍部署在天梯实例上**（`scripts/probe_min.ps1 restore` 可还原）。

### 批量重演与 HF 数据集（2026-11-21）
- 转换器改为两层：数据源 → "对局规格"（`spec_from_royaleapi` / `spec_from_hf`）→ 原生 JSON（`build_from_spec`）。
- **严格保留原局方位**：side 按 data-i 标记（team = 1 - data_i），坐标用原始 RoyaleAPI 坐标。镜像或旋转都会改变结局（HF 20 局：原始坐标 9/20，HF 的 native_world_units 3/20，旋转 180° 2/20）。"野猪方在下方"改到生成训练数据时再旋转。
- 英雄技能：`ct=189`，`cgid` = 场上英雄/冠军单位的对象 ID（实体 `+0x08`）。批量脚本逐个补全技能：重演 → 在技能 tick 找该方英雄/冠军单位 → 补上 → 再重演。英雄单位编号不一定是 203000000 + 卡牌编号（英雄狂战士 26000102 → 203000076），找不到候选时退回该方任意英雄或冠军单位。
- 起手顺序：找不到完全合法的顺序时，取能让最长出牌前缀合法的那种。
- **结局一致率**：
  - 我们的 72 局：皇冠完全一致 47/72，胜负一致 55/72（`scripts/batch_headless.py`）；
  - HF 40 局：皇冠完全一致 25/40，胜负一致 29/40（`scripts/hf_batch.py`）；技能补上 153/178。
  - tick 偏移 ±1 不影响一致率。剩余偏差估计来自原局随机种子和其他未公开状态。
- 修复：引擎每局结束后释放管理器和 JSON（之前泄漏，跑几百局后系统重启）；代理库等主界面就绪后才接任务，并删掉启动时残留的任务文件。修复后 RSS 稳定在约 530MB。
- HF 数据集：先下载了 1 个分片（`data/hf_il_replay/replays/part-000000.parquet`，5000 局，其中野猪 2.6 有 210 方）。全量 52 个分片（787MB）估计约 1.1 万方野猪 2.6；和用户形态完全相同的约 14%。

### 全量野猪 2.6 重演（2026-11-21 11:35 开始，后台运行中）
- HF 全量 52 个回放分片已下载并校验（`scripts/hf_download.py`，`data/hf_il_replay/replays/`）。
- 后台任务：`scripts/hf_resim_hog.py`（PID 记在 `re/hf_resim_hog.pid`，日志 `re/hf_resim_hog.log`），可断点续跑，重新运行即可继续。
  - 输出：`data/resim_headless/hog/<tag>.jsonl.gz`（逐 tick 帧，每局约 230KB）、`<tag>.native.json.gz`；汇总 `data/resim_headless/hog_summary.jsonl`。
  - 进度：`python scripts/hog_progress.py`。
  - 约 4.7 秒一局，估计约 1.1 万局、14–15 小时。前 73 局：全部成功，皇冠一致 53/73，胜负一致 57/73。
- 稳定性：模拟器虚拟机曾自行停掉（adb 无设备，批量任务卡住）。已加 adb 超时，设备不见时用 `ldconsole quit/launch --index 0` 自动重启；游戏没在运行时自动启动并等代理库就绪。
- **运行期间不能用这台模拟器打天梯。**

### 模仿学习 v2（2026-11-21，流程已打通，等数据）
- 数据集 `crai/data/dataset_v2.py`：`python -m crai.data.dataset_v2`（增量，只处理新完成的对局）→ `data/il_v2/<tag>.npz` + `vocab.json`。
  - 视角：野猪方，统一旋转到己方在下方（y 大）。每 5 tick 一个决策点；标签来自原生指令（下达 tick、手牌槽位、18×32 格子；技能单独一类）。
  - 特征：时间/阶段、双方圣水、双方塔血、双方出牌数、双方手牌和下一张（对手的也能从内存读到）、最多 40 个单位（编号、阵营、位置、血量比例、最大血量、kind）。
  - 结局与原局不一致的对局，截到第一条未执行指令之前。
  - 每局约 0.12 秒。
- 模型 `crai/model/policy_v2.py`：Transformer，token 为全局 + 10 张手牌 + 单位；输出不出/出牌/技能、手牌槽位（与手牌 token 匹配）、落点 576 格（以所选手牌为条件）。
- 训练 `scripts/train_v2.py [轮数] [keep_noop]`：按对局划分验证集，"不出"样本下采样并加权，GPU 上每轮约 1 秒（170 局时），输出 `models/hog26_v2.pt`。
  - 170 局时的验证结果：出牌时机 AUC 0.66，出哪张 52%，落点 1 格内 37%；几轮后就不再提升，说明数据不够。
- 实战 bot `crai/bot/v2_bot.py`：`python scripts/run_bot.py N training v2`。每 0.25 秒决策；非法术卡限制在己方半场；技能暂不执行。
  - 实时读取器的 Entity 新增 `raw_card`（原始编号，英雄单位为 203xxxxxx），v2 用原始编号。
  - `tests/test_v2_features.py`：用重演帧冒充实时帧，确认实战特征与训练集逐项一致（通过）。

- 第一次天梯实测（2026-11-21，约 200 局训练的模型）：0:3 输。模型实战出牌概率只有 0.02–0.13，几乎全靠"圣水满了强制出牌"；落点差（滚木下在自家桥头、冰精灵下在最后一排）。实战特征与训练分布的差异待诊断（可能是用墙钟估算的时间）。
- 训练脚本改进：载入时只保留 30% "不出"样本（全量时内存约 2–3GB）；训练结束在验证集上自动校准出牌阈值，存进模型文件，v2 bot 自动读取。

- 第二次天梯实测（2026-11-21 13:33，约 1400 局训练：AUC 0.71、出哪张 54%、落点 1 格内 39%）：0:3 输，出牌仍几乎全靠"圣水满了"。
  - `scripts/diag_v2_live.py`：实战特征分布与训练一致，没有 bug。
  - 根因：真人在任意 0.25 秒窗口内出牌的概率本来就只有 4–7%（即使圣水满），模型已校准，所以纯模仿学习分不清"该出的时刻"；进攻、防守都差。
  - FirstLight 的经验也一样：25 万局模仿学习只有"实力一般"，名人堂模型靠 PPO 自我对打。
  - **路线建议（待用户决定）**：A. 写适配器接 FirstLight 野猪专精模型（推荐，最快变强）；B. 把离线引擎做成逐步推进的环境，自己做 PPO。A 和 B 都需要"逐步推进 + 中途下指令"的离线引擎来做离线评测。

### 可交互离线引擎（2026-11-21 下午，已完成）——路线 A、B 的共同基础
- 用户决定 A、B 都做（A：接 FirstLight 野猪专精模型；B：自己做 PPO）。
- 代理库新增 TCP 服务（游戏进程内，127.0.0.1:26789，主机 `adb forward`）：`RESET`（开局 JSON）/ `CMD`（注入一条原生指令）/ `STEP n`（推进 n tick，返回当前帧和 ended）/ `CLOSE`。
  - 指令注入：JsonParse → LogicCommandParse `0xe553e4`（x8 返回结果，经 `crai/probe/native_call_arm64.S` 调用；上下文 = `*(管理器+0x210)`，15 版 `+0x208`）→ LogicCommandAppend `0xe5443c` 到 `*(管理器+0x38)`。
  - 与旧的文件触发批量重演共用引擎，互斥锁保护。
- 客户端 `crai/env/native_env.py: NativeEnv`（reset/inject/step/close，`play_cmd`/`ability_cmd` 生成指令）。
- 自检 `scripts/env_selftest.py`：ladder_last 一次性载入 vs 在下达 tick 逐条注入，3107 帧**逐帧完全一致**；每步往返约 1.6ms，逐 tick 推进一局约 5 秒。

### 离线对战评测（2026-11-21 下午）
- `crai/env/match.py: play_match(agent0, agent1, deck0, deck1, seed)`：两个 AI（实战 bot 接口 `decide(frame, now) -> Action`，己方在下方视角）在交互式离线引擎里对打；负责换算卡组位置、pd 编码、引擎坐标；记录出牌和被拒次数；按塔数算皇冠。
- `scripts/eval_match.py <A> <B> [局数]`：交替换边、每局换随机种子。AI：`rules`、`v2`、`idle`。
- 基线（双方野猪 2.6，每局约 1–5 秒）：
  - rules vs idle：2:0（6:0 皇冠），无被拒。
  - **v2（约 1400 局训练）vs rules：4 胜 6 负，皇冠 6:12**。
- FirstLight 模型能在我们的 venv（Python 3.13、torch 2.11）里正常加载（野猪专精 2：1200 万参数，3 秒）。
- 子任务正在整理 FirstLight probe 的 raw 局面格式（`docs/fl_raw_schema.md`），用于复用其 BattleEnvV1 构建 ObservationV1。

### A1 完成：FirstLight 模型接入离线引擎（2026-11-21 晚）
- 引擎帧新增：手牌 `pd`（游戏函数 CardSelection::build `0xf671a0` 现算，x8 返回，经汇编桥调用）、账号 `acct`、单位 `dgid`（数据全局 ID = `*(实体+0x48)+0x40`，15 版是 `+0x10`；与 FirstLight 目录一致，例：野猪 34000021、英雄火枪手 130283371、进化加农炮 140000000）、`others`（投射物、区域效果）。
- 修复：进程里可能有两份 libg（崩溃上报 SDK 映射了只读副本），代理库只认"有可写数据段 + GameStateStep 开头字节一致"的那份；不再用 dl_iterate_phdr。读未知内存改用 process_vm_readv 安全读。
- 适配器 `crai/env/fl_agent.py: FLAgent`：引擎帧 → FirstLight raw（`to_fl_raw`）→ 子类化 BattleEnvV1（合成空 rich telemetry，自己补双方 action_executed 事件）→ PolicySessionV4 → 原生出牌指令。技能暂时被屏蔽（未提供 ability runtime）。
- `play_match` 支持 `decide_raw(raw, acct)` 型 AI；`scripts/eval_match.py fl[-hog1|-general|-il] …`。
- **离线评测（双方野猪 2.6，交替换边，CPU 推理每局 40–80 秒）**：
  - FirstLight 野猪专精 2 vs 规则 bot：**10:0**，皇冠 10:0；
  - FirstLight vs v2：**10:0**，皇冠 13:0；
  - 20 局零被拒、零失塔。

### A2 进行中：FirstLight 模型打实战（2026-11-21 晚）
- 实时对局帧：tick = `*(*(世界+0x8)+0x60)`；代理库新增 `LIVE <世界对象地址>` 命令（地址来自实时读取器帧的 roster），输出与离线引擎相同格式（含手牌 pd、dgid）。
- `scripts/run_fl_live.py [局数] [training|ladder] [模型]`：FLAgent 决策 → 拖拽下牌。下牌后等出牌序号增加（或 1.5 秒）再执行新的出牌；单帧出错只记录不中断；对象按 ID 去重、血量限制为非负。
- 结果：
  - 训练营 1 局：**3:0 胜**，零失塔。
  - 天梯第 1 局：第 150 秒因负血量报错退出（当时 0:0），之后无人操作，加时 0:1 负（已修复）。
  - 天梯第 2 局：完整 205 秒无错误，出牌 40 次，**加时 0:1 负**。打法偏保守，3 分多钟只进攻 3 次野猪。
- 待改进：实战有约 1 秒下牌延迟（20 tick 生效 + 点击耗时），主分支模型按约 1 tick 训练 → 试 yes-delay 分支的 Everlight；技能未启用（英雄火枪手技能）。
- **Everlight**（`refs/FirstLight_CR_yesdelay`，按 20 tick 延迟训练的通用模型）：`CRAI_FL_ROOT=yesdelay` 切换分支（两个分支包同名，一个进程只能用一个），模型名 `everlight`。
  - 需要"在途动作"：每次下牌往 env._pending 登记 `_PendingRecord`，执行后清除，超时 40 tick 清除；指令 `t = 当前 tick`，`t2 = t + 延迟档位 + 20`。
  - 面对空场会一直等：兜底在可出牌后 15 秒强制出第一张（同 FirstLight 评测协议）。
  - 离线：vs rules 2:0（4:0），vs v2 4:0（6:0）。
- 修复：对手出镜像（Mirror，28000006）时效果牌取其上一张非镜像牌（否则 FirstLight 每帧报错）；实战连续 10 帧出错就从当前帧重建智能体。
- **天梯 4 局全负**：野猪专精 0:1（中途崩溃）、0:1；Everlight 0:3（镜像 bug 停摆）、0:1（完整零错）。完整打满的两局都是加时只差一塔。
- **实战精度改进（2026-11-21 晚）**：
  - 更正：FirstLight 的 `nOnTouchEvent` 钩子只用于它自己的离线人机界面，**没有**实战直接调用游戏出牌函数的实现；不走构造出牌指令发服务器这条路（风险高）。
  - 常驻触摸注入 `crai/tools/touchd.c`（直接写 `/dev/input/event2`，多点触控 Protocol B）：LDPlayer 设备坐标 X 0..1919 ↔ 屏幕 X 0..1080、Y 0..1079 ↔ 屏幕 Y 0..1920（按轴缩放，不旋转；用 dumpsys input 对照确认，误差 1–2px）。`crai/client/control.py: TouchD` 自动启用，失败时退回 `input`。选牌+点击 281ms → 65ms。
  - 标定：`scripts/collect_calib.py` 随机目标收集样本（68 → 108），二次模型留一误差中位 20px；验证命中 6/11 → 8/10。
  - 对手用 FirstLight 不认识的 16 版新卡（如 26000107）会让适配器建立对局配置就报错、整局空转（天梯一局 0 出牌 0:3）→ 换成同费用替身卡；实战连续重建 3 次仍失败就由规则 bot 接管。
  - 天梯：Everlight 又 0:3（新卡空转）、0:1（零出错）。**FirstLight 天梯累计 0 胜 6 负**，完整打满的 4 局有 3 局 0:1。
- 原计划的下一步（实战精度）：① 像 FirstLight 一样在游戏线程里调用游戏自己"完成玩家出牌"的函数，代替 adb 拖拽（目前落点约 21% 偏 1 格，拖拽还有几百毫秒延迟）；② 启用英雄技能；③ 多打几局统计胜率。

### 总路线（用户确认，2026-11-21 晚）
终极目标国际服名人堂。阶段 0：Null's 读内存 + 回放重演精确学习、离线模拟训练（PPO），Null's 天梯验证，目标名人堂；
之后阶段 1 观测降级到"视觉可见"信息 → 阶段 2 用 Null's 截图 + 读内存配对自动标注训练视觉 → 阶段 3 影子模式切换 → 阶段 4 国际服（只用视觉 + 触摸）。
- 已修：`run_fl_live.py` 以前一个决策点只执行第一条出牌（`break`），FirstLight 的"第二个动作"（如野猪 + 火枪手同时下）被丢掉；现在同一决策点的出牌全部执行（日志 `n_cmds`）。待实测。
- 发现的错配：野猪专精模型（main 分支）按"约 1 tick 生效"训练，实战是 20 tick + 点击延迟 → 计划用离线环境以 20 tick 延迟做 PPO 微调野猪专精模型（阶段 0 核心）。
- 英雄技能仍被屏蔽：需要技能就绪状态（ability_runtime）+ 原生技能指令（ct=189）。

### PPO（阶段 0 核心，2026-11-21 晚开始）
- **重要发现**：以前离线评测里主分支 FirstLight 模型的出牌指令被"倒填时间"（t = t2 − 20），几乎立刻生效；规则 bot 和实战都是 20 tick 后生效。所以以前离线"10:0 赢规则 bot"占了延迟便宜。按真实延迟：迁移后的野猪专精 2 号 vs 规则 bot 4:0，但每局只赢 1 冠；vs Everlight（卡组池卡组）冒烟测试 0:2。
- FirstLight 训练记录：作者用野猪专精模型上了名人堂；主线配方 = 胜负 + 塔血势能（β 0.05）+ 圣水溢出惩罚，对手 40% IL / 60% General，lr 1e-5、γ 0.9997、λ 0.98、clip 0.1、target KL 0.01、2 轮。
- 迁移：`scripts/fl_migrate.py`（主分支 → yesdelay：卡牌运行时特征 16 → 21 补零，补齐新增配置开关，写入 delay_policy earliest / 延迟上限 200ms）→ `models/ppo/hog2_yesdelay_init.pt`。
- 对手卡组池：`scripts/build_deck_pool.py` → `data/ppo/deck_pool.json`（HF 1.4 万局野猪局的对手方卡组，4275 种，按次数加权）。
- 训练：`crai/rl/ppo_fl.py`（采样记录会话、奖励、组装 StoredRolloutV4、调用 FirstLight 的 distributed_ppo_update_v4 单进程）+ `scripts/ppo_train.py [更新次数] [每次局数] [输出目录]`。
  - 必须 `CRAI_FL_ROOT=yesdelay`、`CRAI_INSTANCE=1`，用 `.venv\Scripts\python.exe`。
  - 对手池：规则 bot 20%（野猪镜像）、Everlight 40%（卡组池）、冻结的初始模型 40%（一半卡组池一半镜像）。
  - 奖励：胜负 ±1 + 塔血势能 0.05 + 圣水溢出（整局超过 4 点后每点 −0.01）。
  - 每局记录对手出牌数，"对手出牌 ≤ 3"的局单独统计（防钻空子）。
  - 游戏崩溃（与批量重演同实例并发时偶发，引擎空指针）→ 这局作废，`ensure_game` 重启后重连。
  - 断点续跑：输出目录有 `latest.pt` 就从它继续（含优化器）；每次更新另存 `update-xxxxx.pt`。
- 冒烟测试通过（`models/ppo/smoke`）：价值网络解释方差 0.79，approx_kl 0.0136，一局约 1 分钟，一次更新约 20 秒（GPU）。
- **正式训练 run1**：`models/ppo/run1`（200 次更新 × 8 局，PID 在 `train.pid`，日志 `train.log`、`episodes.jsonl`、`updates.jsonl`），2026-11-21 20:38 开始，每次更新约 9 分钟。
- 固定评测：`scripts/ppo_eval.py <检查点> [每种对手局数]`（argmax、固定种子 20261121、固定卡组池卡组，默认实例 0，一轮 30 局约 16 分钟）→ `models/ppo/eval/*.json`。
  - 自动评测：`scripts/ppo_eval_watch.py`（后台，PID 在 `models/ppo/eval/watch.pid`，每 10 次更新评测一次）；看汇总表：`.venv\Scripts\python.exe scripts/ppo_eval_watch.py table models/ppo/run1`。
  - **基线（起点模型，真实 20 tick 延迟）**：vs Everlight 1-0-9（皇冠 1:11），vs 起点模型自己 4-1-5，vs 规则 bot 10-0-0；30 局无消极对手。Everlight（按延迟训练的通用模型）远强于延迟下的野猪专精。
  - 修复（2026-11-21 晚）：`card == -1` 的不只是塔（对局中途生成的实体也是），曾出现负皇冠、奖励错误 → `crai/env/match.py: TOWER_POS / is_tower` 按 6 个塔的固定坐标认；`ppo_fl.frame_info`、`headless_resim.summarize`、`run_fl_live` 同步修改。旧的 3 次更新存档在 `models/ppo/run0_bad_reward`。
  - 对照：Everlight 拿野猪卡组 vs Everlight 5-0-5，vs 起点模型 5-0-5（其中 3 局 Everlight 整局只出 1–2 张牌卡住），vs 规则 bot 10-0-0 → 延迟下 Everlight 打野猪比起点模型强；可作为备选起点。
- **加速（2026-11-21 晚）**：
  - 剖析一局 74 秒：模型推理 69%、引擎往返 22%。引擎本身不是瓶颈（规则 bot 对打 4 连接 46 局/分钟）。
  - 代理库 TCP 服务改为多客户端（每连接一个线程、各自一局，最多 24 个，断开即销毁该局），已部署到实例 1（实例 0 还是旧版，单连接用法兼容）。`scripts/probe_min.ps1` 加 `-Serial`，并修复 UTF-8 BOM / adb stderr 问题。
  - `FLAgent.decide_raw` 拆为 `prepare` / `finish`；`fl_agent.batch_decide` 多局观测拼批一次推理（动作一次性回 CPU、整批校验；批量时不用 CUDA Graph —— 它的逐字段拷贝 + 同步曾占 70%）。批大小 1 时与逐局结果逐局一致；多局拼批因补齐导致浮点微差，结果会分叉（不影响训练）。
  - `crai/env/match.py: MatchState`（逐步推进的单局）+ `crai/env/vec_match.py: run_many`（一个进程 N 局，批量推理，线程并发 STEP，崩溃时整批作废重连，单局出错只作废该局）。
  - 实测（截短到 1500 tick）：单进程 8 局 5.5、20 局 7.2；2 进程 × 10 局 10.0（最好）；3 × 7 局 9.1（GPU 饱和）局/分钟。
  - `scripts/ppo_train.py` 改为 2 个采集进程 × 10 局并行（`CRAI_PPO_WORKERS` / `CRAI_PPO_PARALLEL`），每次更新 20 局：约 9 分钟/次（采集 ~8 分钟 + 更新 ~1 分钟），完整对局约 2.2 局/分钟（以前 0.8）。
  - 训练卡组池只保留 FirstLight 认识的卡（4202 种；16 版新卡如 26000105 会让 tensorizer 报错）；评测卡组固定按最初规则抽取（`ppo_eval.eval_decks`，与基线一致）。
  - 下一步提速：批量推理也用 CUDA Graph（固定批大小与补齐容量），预计还能快数倍。
- run1 进度：旧版（单局）完成 617–625（9 次 × 8 局）；新版从 626 起（PID 在 `models/ppo/run1/train.pid`，日志 `train3.log`）。
- **天梯实测（2026-11-21 23:22，update 626，约 92 局 PPO）**：加时 0:1 负（己方 2 塔、对方 3 塔），69 次出牌零出错。野猪 13 次（11 次左桥，比主分支模型积极），但冰精灵 8 次、骷髅 4 次下在己方国王塔正后方；首野猪 72 秒才下。用户评价：进攻、防守都不太行。日志 `data/bot_runs/fl_20261121_232157.jsonl`。
- 待做（按优先级）：
  1. 看过夜训练的离线评测汇总表，判断 PPO 是否有效（对 Everlight 从 1-9 往上走）；无效就考虑以 Everlight 为起点做野猪专精。
  2. 英雄火枪手技能（实战等于少一张牌的能力）。
  3. 批量推理用 CUDA Graph 再提速。
  4. 锚定 KL（防遗忘）。
  5. 实战之前先在离线评测里达标，再上天梯多打几局。
- 后台（过夜）：实例 1 训练 run1（`models/ppo/run1/train.pid`）、全量重演（`re/hf_resim_hog.pid`）；实例 0 自动评测（`models/ppo/eval/watch.pid`）。停止：对这些 PID 及其子进程 Stop-Process。**自动评测在用实例 0 的引擎，打天梯前要先停掉它。**

### 2026-11-22：参考仓库落地（见 docs/references.md"新参考仓库评估"）
- **15.535.13 不能联网**：新建 LDPlayer 实例 2（`crai_v15`，由实例 1 复制），装旧 APK 后启动即弹"可以更新啦！"，只有"更新"按钮，随后游戏退出。继续用 16.635.1。实例 2 已关闭，可删。
- 过夜任务（PPO run1、全量重演、自动评测）在 11-21 23:29 因模拟器重启失败全部停止；run1 最后是 update-627。
- clashroyal-replay：HF 数据已带它从网页解析的元数据（形态、等级、塔兵、六塔终局血量、溢出圣水），`spec_from_hf` 已在用形态、等级、塔兵。可移植的只剩"用终局塔血 + 溢出圣水给技能归属打分搜索"（提升重演一致率，暂缓）。它的发布 APK 是腾讯官服回放控制台，Null's 用不上；源码包下载到 `refs/new2611/clashroyal-replay-release/`。
- **代理库新增字段（已部署实例 0；实例 1 还是旧版）**：
  - 单位 `"atk":[目标 gid, 阶段, 时间轴 ms, 装填 ms]`：攻击组件 = 组件数组 `+0x18` 的第 0 个（`comp+0x08 == 实体`），`+0x10` 目标指针、`+0x20` 阶段、`+0x24` 时间轴、`+0x28` 装填，与 15 版一致。离线实测目标 100% 解析为敌方对象（野猪先锁塔，被加农炮吸引后改锁加农炮）。
  - 玩家 `"abil":[{s, st 按钮状态, cd, tot, chg 次数, adg, cdg, n 技能名, m 成员 gid, mc}]`：控制器在 `player+0x3a0/+0x3a8`（与 15 版相同），`+0x20` 回指 player，`+0x78/+0x7c/+0x80` 冷却/总冷却/次数，`+0x90` 角色数据（`+0x710` → 技能数据名字），`+0x98` 按钮状态（1 无英雄、2 可用、9 暂不可用、6 已释放），`+0xa0` 成员向量（`+0x00` 数组、`+0x0c` 数量）。技能数据里的最大次数和配置冷却偏移变了，改用目录值。
  - 调试命令 `PEEK <地址> <字节>`、`WADDR`（`NativeEnv.peek` / `world_addr`）。
- **FLAgent**：
  - 攻击目标和阶段填进 rich telemetry（`target_entity_key` / `attack_sequence_stage`）：观测里 75% 单位有 `visible_target`、84% 有 `attack_state`（以前都是 0）。
  - 英雄技能：rich 玩家数据填 `ability_runtime`，`_NullNative` 加占位的 `queue_ability_action_at` 放开掩码；`finish` 把技能动作转成 `ct=189`（cgid = 英雄单位对象 ID）。离线验证：时机采样下模型放了技能，引擎执行成功（次数 1→0）。全 argmax 时模型从不放技能（370 个可放的决策点 0 次）。开关：`CRAI_FL_ABILITY=0` 关闭技能。
  - `decide_raw(..., allow=False)`：实战中上一次点击没确认时不调用模型（RoyaleHarness 做法），被丢弃的动作不再写进模型历史和在途牌。
  - 实战脚本遇到技能指令先跳过并记录（技能按钮还没标定）。
- 温度 A/B：`scripts/temp_ab.py <检查点> [局数] [argmax,gate,full]`，对 Everlight，同一组种子和卡组。update-627（实例 1，旧代理库）：argmax 4-0-36，gate（时机 T=1、选牌 T=0.05）1-36（37 局时），full 见 `models/ppo/eval/temp_ab_run1_update-00627.json`。
  - 结果（各 40 局）：argmax 4-0-36（皇冠 8:43）；gate 1-0-39（4:65）；full 0-0-40（5:62）。**对 FirstLight 野猪模型，argmax 最好，实战继续用 argmax**（Battle-Engine 的温度结论不适用）。
- 新观测下（攻击目标 + 技能，双方都有）对 Everlight 各 40 局 argmax：update-627 4-0-36（皇冠 9:38）；Everlight 拿野猪卡组 5-2-33（8:48）。说明这个评测基准对野猪卡组本身就很难，区分度有限；PPO 10 次更新没有可见提升。
- 训练营实测 1 局（实例 0 新代理库，update-627）：3:0 胜，零出错，实时帧带上新字段正常。
- 实例 0、1 都已部署新代理库。
- `scripts/block_gamepad.ps1` 加 `-Serial`；实例 0、1 都已屏蔽手柄。

### 2026-11-22 下午：训练方向调整——只从人类回放和天梯实战学习（用户决定）
- 用户：天梯随便测（私服）；英雄技能靠 PPO 学；训练数据只用 RoyaleAPI/HF 人类回放和天梯实战。实例 2 已删除。
- **天梯学习** `scripts/ladder_learn.py [局数] [每次更新局数 8] [输出目录 models/ladder/run1] [起点]`（实例 0，后台，PID 在 `models/ladder/run1/train.pid`，日志 `train2.log`）：
  - 天梯上用 T=1 采样打真人（PPOLearner + RecordingSession），每局轨迹存 `episodes/<时间>.pt`，每 8 局一次 PPO 更新（lr 1e-5，与离线 PPO 相同的奖励），然后一步人类回放行为克隆；存 `latest.pt` / `update-xxxxx.pt`，日志 `games.jsonl`、`updates.jsonl`。断点续跑：`state.json` 记录已用过的对局。
  - 起点：`models/ppo/run1/update-00627.pt`。
  - 实战技能：按 `crai/client/control.py: ABILITY_PX = (974, 1450)`（来自 RoyaleHarness 参考值），按"技能次数下降"确认；已确认能放出来（tick 3430 按下，23 tick 后生效）。
- **人类回放行为克隆**：
  - `scripts/resim_v2.py`：用新代理库（atk、abil）把第一轮重演中结局完全一致、用户形态的 2445 局，按第一轮的原生 JSON 经 TCP 重新录逐 tick 帧 → `data/resim_headless/hog_v2/`，汇总 `hog_v2_summary.jsonl`（后台，实例 1，约 2.6 秒/局；PID `data/resim_headless/resim_v2.pid`）。抽查与第一轮结局 100% 一致。
  - `crai/rl/expert_data.py: build_sequence`：重演帧逐帧喂给和实战相同的适配器（FLAgent.prepare），每个决策点用 FirstLight 的 `build_expert_action_batch` 把人类在 5 tick 窗口内的指令对齐成标签；人类动作写进动作历史、出牌登记在途牌。每局约 8 秒，约 95% 有动作的窗口能对齐；落点标签 100% 在合法格子内。序列约 120MB/局，不存盘，训练时现场构建。
  - `ExpertSampler` + FirstLight `distributed_expert_bc_update_v4`：每次随机 8 局野猪方赢的局，系数 0.2，延迟头和价值头权重 0（earliest 延迟策略）。单步测试 2 局 78 秒、显存 0.8GB。

### 2026-11-22 傍晚：再调整——训练用离线引擎，天梯只做验证（用户："多用离线引擎，验证用实战天梯"）
- 天梯学习（ladder_learn）已停，`models/ladder/run1/episodes/` 保留 7 局轨迹（3 局 0:1、0:3 等，1 局 1:0）。
- **离线 PPO run2**（实例 1，后台，PID `models/ppo/run2/train.pid`，日志 `train.log`）：
  `ppo_train.py 300 20 models/ppo/run2 models/ppo/run1/update-00627.pt`。与 run1 的区别：观测带攻击目标、技能控制器（新代理库），技能放开（PPO 采样可以探索技能），每次更新后一步人类回放行为克隆（`CRAI_BC_COEF` 默认 0.2，`CRAI_BC_GAMES` 默认 8）。对手池不变（规则 20% / Everlight 40% / 冻结起点 40%）。
- **天梯验证** `scripts/ladder_validate.py models/ppo/run2 10 6`（实例 0，后台，PID `models/ppo/run2/ladder.pid`）：起点（复制进 run2 的 update-00627）先打 6 局，之后每 10 次更新用最新检查点 argmax 打 6 局。结果 `models/ppo/run2/ladder_eval.jsonl`，汇总 `python scripts/ladder_validate.py table models/ppo/run2`。

### 2026-11-23：run2 效果评估——无效，反而退化
- run2 跑到 update 747（11-23 07:26 停，进程已不在），共 3144 局训练局。
- 训练局胜率（每 20 次更新一段）：对冻结起点 18% → 4% → 7% → 7% → 5% → 3%；对 Everlight 一直 1–4%；对规则 bot 87–97% 不变。
- 末段停摆局（出牌 ≤5）从 0–2 升到 10，圣水浪费回升到 8.8；策略熵 0.24 → 0.32。
- 天梯验证 14 个检查点 19 局，只有 727 赢 1 局，无上升趋势。
- 判断：对 Everlight 几乎全输，奖励信号几乎恒为 −1；对规则 bot 几乎全赢，也没信息；中间难度对手缺失。训练时 T=1 采样本身比 argmax 弱很多（温度 A/B），对 argmax 起点也吃亏。

### 2026-11-23：run3——起点 Everlight + 联赛式对手池（用户同意 A+B）
- `scripts/ppo_train.py` 对手池改为 `CRAI_OPPONENTS`（默认 league 0.6 / snapshot 0.15 / everlight 0.15 / rules 0.1）：
  league = 输出目录里自己的历史检查点（一半取最近 `CRAI_LEAGUE_RECENT`=5 个，一半全部历史均匀取）；模型对手默认也采样出牌（`CRAI_OPP_SAMPLE=1`），和学习方对称。还没有检查点时 league 退回冻结起点。日志多了 `opp_tag`（对手检查点名 / init / everlight / rules）。
- 保存检查点时沿用起点自带的 extra（Everlight 有 allowed_delay_offsets_ms）。
- `scripts/run_bg.ps1` 的 train 改成 run3：`ppo_train.py 20 20 models/ppo/run3 <Everlight>`，先短跑 20 次更新（update 80 → 100），天梯验证仍由训练脚本每 10 次更新在实例 0 发起。
- 早上模拟器被外部重启，两个实例都没开 → run2 停止的原因。已用 `headless_resim.ensure_game` 拉起两个实例，并重新屏蔽手柄。
- 第一轮（30 局）：对冻结起点 9 胜 12 负（43%，信号正常），对 Everlight 0/6，对规则 bot 2/2。
- 判断标准：对冻结起点的胜率持续高于 50%，而且对 Everlight 和天梯不下降，才长跑。

### 下一步
A2. 接实战天梯：代理库在实时对局里也能取到同样格式的帧（手牌 pd、dgid），FLAgent 决策 → 点击下牌。要处理：实时对局的 world 对象定位、20 tick 下牌延迟（主分支模型按约 1 tick 训练；可试 yes-delay 分支的 Everlight）、决策节奏与 tick 对齐、点击延迟。
B1. 用离线环境做 PPO（对手池：规则 bot、v2、FirstLight）。
A1. （已完成）读 FirstLight 的 `battle_env.py` / `tensorizer.py` / `policy_session.py`，写适配器：我们的帧 → FAIR `ObservationV1` + `EpisodeConfigV1`；`ActionV1` → 原生指令。先在离线环境里让 FirstLight 野猪模型打规则 bot / 我们的 v2 模型，看胜率。
A2. 离线验证够强后，接实时读取器上天梯（实战还要补 20 tick 下牌延迟：优先试 yes-delay 分支的 Everlight）。
B1. 在离线环境上做 PPO：对手池（规则 bot、v2、FirstLight），奖励=塔血差+胜负。
0. 等全量重演完成（`python scripts/hog_progress.py`）→ `python -m crai.data.dataset_v2` → `python -u scripts/train_v2.py 30`。
1. 暂停批量重演（结束 `re/hf_resim_hog.pid` 进程），用 `run_bot.py N training v2` 在训练营实测，再上天梯。
0.5 用重演数据接 FirstLight 的模型，或先做模仿学习；天梯适配器。
1. 核对 6 个 check 地址和 `struct_offsets_to_verify` 里的结构体字段。
1.5 ~~验证 LDPlayer 上能否做运行时 inline hook~~ **已验证可行（2026-11-21）**：
   - 方法：**不改 APK**，root 下把已安装的 `lib/arm64/libsentry-android.so` 换成代理库（`crai/probe/proxy_min.c`），原库改名 `libsentry-android-real.so` 由代理库的 DT_NEEDED 加载。签名不变，账号数据不受影响。脚本 `scripts/probe_min.ps1 build|deploy|restore|log`。
   - 代理库先于 libg 加载（Java 层 sentry 先 loadLibrary），且转译环境下 `dl_iterate_phdr` 看不到 libg → 后台线程轮询 `/proc/self/maps` 等 libg 出现再装钩子。
   - GameStateStep（`0x11d3174`）钩子生效，主界面每秒约 20 次调用（地址也因此确认）。**天梯实战 1 局（规则 bot，输）全程稳定，进程未重启**。对战中 Step 没有被调用，说明实时对战走的是另一条路径，GameStateManager 只在主界面和回放里跑。
   - 测试完已 restore 回原库。
2. 第二个 LDPlayer 实例 + 打补丁的 APK，先只做最小 probe：创建 GameState、注入指令、逐 tick 推进、导出状态。用 replay0013 对比游戏内重演结果。
3. 用 FirstLight 的 resource_compiler 从 16.635.1 的 CSV 重新生成卡牌目录。
4. 天梯适配器（子任务评估约 2–4 周）：落点掩码、手牌形态码、出牌事件合成、实体 archetype id 对齐、20 tick 下牌延迟（优先用 yes-delay 分支的 Everlight 模型）。

## 原定待办（暂停，按顺序）
1. **（需要用户同意）打 1 局天梯制作大容量底子**：bot 一直下 1 费卡（骷髅兵、冰精灵），尽量拖进加时，目标 200 条以上指令。之后把这局放在对战日志最上面当底子（注入脚本默认点第 1 局的"观看"）。
2. 批量重演全部 72 局：`python -u scripts/batch_resim.py 200`，统计结局一致率。
3. 分析不一致的局：英雄技能、进化、tick 的含义（RoyaleAPI 的 data-t 对应生效 tick 还是下达 tick）。
4. 用 `data/resim/*.jsonl` 的完整战局数据，加上实时单位位置和血量，重新构建模仿学习数据集，重点让模型学会落点，然后上天梯测试。

## 环境提醒
- **两个 LDPlayer 实例（2026-11-21 晚）**：
  - 实例 0（emulator-5554 / 127.0.0.1:5555）= 天梯账号，实战、天梯、训练营测试；所有脚本默认连它。
  - 实例 1 `crai_engine`（emulator-5556 / 127.0.0.1:5557）= 由实例 0 用 `ldconsole copy` 复制而来，清过游戏数据（新游客号，弹"登录失败 / Cards unlocked!"也不影响），代理库同样在；专跑离线引擎（重演、PPO）。
  - 选实例：环境变量 `CRAI_INSTANCE=1`（或 `CRAI_SERIAL=emulator-5556`）。`crai/device.py` 不再"取第一个在线设备"。
  - 主机端口：实例 0 转发 26789，实例 1 转发 26790（`crai/env/native_env.py: host_port`，可用 `CRAI_ENV_PORT` 覆盖）。
  - `headless_resim.ensure_device` 只重启自己那个实例。
  - 后台重演要用 `.venv\Scripts\python.exe` 启动（系统 python 没有 pyarrow），例如：`$env:CRAI_INSTANCE="1"; Start-Process .venv\Scripts\python.exe -ArgumentList "-u","scripts/hf_resim_hog.py" ...`。
- **手柄屏蔽**（2026-11-21）：LDPlayer 把电脑手柄转接成 Android 设备 "joystick"（`/dev/input/event5`），窗口不在前台也会收到，用户用手柄玩别的游戏会误操作模拟器。`scripts/block_gamepad.ps1 on|off|status` 用常驻的 `grab_joystick`（EVIOCGRAB 独占该设备）屏蔽；模拟器重启后需要重新 on（批量任务自动重启模拟器时会自动重新屏蔽）。只影响模拟器内部，Windows 上的手柄照常可用。
- **"已授予 Shell 超级用户权限"提示**：来自 LDPlayer 内置的 Koush Superuser（集成在"设置"应用，配置在 `/data/user_de/0/com.android.settings/databases/superuser.sqlite` 的 `settings` 表，空表时默认每次 su 都弹提示框）。
  - `adb root` 不能解决：root adbd 下每个 adb shell（包括截图）都会弹提示，更频繁。代码不再主动 `adb root`。
  - 需要 root 的命令统一走 `crai/device.py: as_root()`。
  - **已关闭（2026-11-21）**：Superuser 的设置页被 LDPlayer 禁用（`Settings$SuperuserActivity` 一启动就退出），改为由用户执行 `scripts/su_toast_off.sh`（只把 `settings.notification` 设为 0，不改授权日志）。恢复：把脚本里的 0 改成 1 再执行。模拟器重装/重置后需要重新执行。
- 模拟器设备名会变（emulator-5554 或 127.0.0.1:5555），`crai/device.py` 会自动检测。
- `su` 调用会在屏幕底部弹提示，界面识别要避开那个区域（已处理）。
- 回放中按返回键会暂停并弹确认框。
- 不要用 `pkill -f cr_reader`，它会把执行命令的 shell 一起杀掉；用 `pkill cr_reader`。
