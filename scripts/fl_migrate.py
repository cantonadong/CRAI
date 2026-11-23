"""把 FirstLight 主分支模型（例如野猪专精 2 号）迁移到 yesdelay 分支格式，用于在 20 tick 下牌延迟下继续 PPO。

yesdelay 分支的 migrate_checkpoint_card_runtime 只允许 card_runtime_feature_dim 不同（16 → 21，补零列），
但 yesdelay 后来还新增了一批模型配置开关（ablate_*、card_id_gate、delay_aware_legality、
neutralize_pending_ticks_remaining），主分支模型文件的契约里没有这些键，于是被拒。
这些开关不改变网络参数（之后迁移函数仍会逐个核对 state_dict 的键和形状），所以这里先把缺的键按当前默认值补上，
再调用原迁移函数（包括它的全部校验和重载核对）。

用法（在仓库根目录）：
  set CRAI_FL_ROOT=yesdelay
  .venv\\Scripts\\python.exe scripts/fl_migrate.py <源 .pt> <目标 .pt>
"""
import json
import os
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FL = ROOT / "refs" / "FirstLight_CR_yesdelay"
sys.path.insert(0, str(FL))

import torch  # noqa: E402
from native_runner.training.v4.checkpoint import checkpoint_contract  # noqa: E402
from native_runner.training.v4.factory import build_production_model_v4  # noqa: E402
from native_runner.training.v4.migrate_checkpoint_card_runtime import migrate_checkpoint  # noqa: E402


def main(src: str, dst: str) -> None:
    model = build_production_model_v4(device="cpu")
    new_cfg = checkpoint_contract(model)["model_config"]
    payload = torch.load(src, map_location="cpu", weights_only=False)
    old_cfg = payload["contract"]["model_config"]
    added = {k: new_cfg[k] for k in new_cfg if k not in old_cfg}
    old_cfg.update(added)
    print("补上的配置项：", json.dumps(added, ensure_ascii=False))
    # 推理设置与 Everlight 一致（checkpoints/README：delay_policy earliest、延迟档位最多 200ms），load_policy_v4 会读取
    extra = payload.get("extra") or {}
    extra.setdefault("delay_policy", "earliest")
    extra.setdefault("inference_delay_offset_max_ms", 200)
    payload["extra"] = extra
    with tempfile.TemporaryDirectory() as tmp:
        patched = Path(tmp) / "patched.pt"
        torch.save(payload, patched)
        summary = migrate_checkpoint(patched, Path(dst), model=model, overwrite=True)
    summary["source"] = src
    print(json.dumps(summary, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
    main(*sys.argv[1:3])
