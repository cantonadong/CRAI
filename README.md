# CRAI

皇室战争 AI：读内存精确学习 → 离线引擎训练（PPO + 人类回放 BC）→ 天梯验证。进度与待办见 [docs/progress.md](docs/progress.md)。

## 项目文件分布

完整项目约 35GB，分两处存放，按目录拼回同一个根目录即可：

| 目录 | 位置 | 大小 | 内容 |
|---|---|---|---|
| `crai/` `scripts/` `configs/` `docs/` `tests/` `replay/` `replay_top200/` `re/`（源码、笔记、小文件） | **本 GitHub 仓库** | ~33MB | 代码、脚本、配置、文档、回放清单 |
| `data/` | HuggingFace | ~8.8GB | 训练数据：PPO 轨迹、resim_headless 重演、IL 数据、royaleapi 回放等 |
| `models/` | HuggingFace | ~21GB | 模型权重：`models/ppo/run*` 各次 PPO checkpoint、`models/ladder`、`hog26_*.pt` |
| `re/` 大文件（`*.gbf` `*.gpr` `*.prp` `*.jsonl` `*.png`） | HuggingFace | ~790MB | Ghidra 工程、逆向导出特征、截图 |
| `build/` | HuggingFace | <1MB | 编译产物（reader/probe 等） |
| `apk/` | HuggingFace | ~870MB | 游戏安装包 `pulled/base.apk`、`libs/arm64-v8a/*.so`、`libs_15535/libg.so`（15 版） |
| `refs/` | HuggingFace（快照）/ 原仓库 | ~1.3GB | 参考项目，见下表 |
| `tools/` | HuggingFace / 官方 | ~1.2GB | `ghidra_12.2_PUBLIC`、`jdk25` |
| `.venv/` | 不上传 | — | 本地 Python 虚拟环境，按 `pyproject.toml` 重建 |

HuggingFace 仓库（私有 dataset）：<https://huggingface.co/datasets/cantonadong/CRAI-assets>
其目录结构与本项目根目录一一对应。

## 拼出完整项目

```powershell
# 1. 代码
git clone https://github.com/cantonadong/CRAI.git
cd CRAI

# 2. 大文件（需登录有访问权限的 HF 账号）
pip install -U huggingface_hub
hf auth login
hf download cantonadong/CRAI-assets --repo-type dataset --local-dir .

# 只要部分目录时用 --include，例如只拿数据和 ladder 模型：
# hf download cantonadong/CRAI-assets --repo-type dataset --local-dir . --include "data/**" "models/ladder/**"

# 3. Python 环境
python -m venv .venv
.venv\Scripts\pip install -e .
```

`hf download` 下到 `.` 后，`data/`、`models/`、`apk/` 等目录会直接落到项目根目录对应位置，与 git 里的文件不冲突。

**注意**：`data/resim_headless/hog/` 有约 1.5 万个文件，超过 HF 单目录 1 万文件的上限，因此以 `data/resim_headless/hog.tar` 存放。下载后需解包：

```powershell
tar -xf data\resim_headless\hog.tar -C data\resim_headless
Remove-Item data\resim_headless\hog.tar
```

HF 默认不收 `.git` 目录，`refs/` 下各参考项目的 git 历史打包在根目录 `refs_git.tar`，解包到项目根即可恢复：

```powershell
C:\Windows\System32\tar.exe -xf refs_git.tar
Remove-Item refs_git.tar
```

（用 Windows 自带的 `tar.exe`；Git Bash 的 GNU tar 会把 `C:` 误当远程主机。）

## refs/ 参考项目原始来源

HF 上的 `refs/` 是当时的工作区快照（含本地改动），`.git` 历史在 `refs_git.tar`。也可从原仓库 clone 并切到对应 commit：


## tools/ 官方来源

- Ghidra 12.2：https://github.com/NationalSecurityAgency/ghidra/releases
- JDK 25：https://jdk.java.net/25/ 或 https://adoptium.net/
