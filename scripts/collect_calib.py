"""连续开训练营，用随机目标格收集下牌标定样本（追加到 re/calib_samples.json），最后重新拟合并报告命中率。

用法：python -u scripts/collect_calib.py [局数 4]
"""
import random
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import start_training  # noqa: E402
import verify_deploy  # noqa: E402


def main(n="4"):
    sys.stdout.reconfigure(line_buffering=True)
    rng = random.Random(42)
    for g in range(int(n)):
        # 己方半场随机目标（避开塔占位附近的最后一行和河道）
        verify_deploy.TARGETS = [(rng.randint(0, 17), rng.randint(17, 30)) for _ in range(16)]
        if start_training.main("training") != 0:
            print("开局失败，跳过")
            continue
        print(f"== 第 {g + 1} 局")
        verify_deploy.main()
    print("重新拟合：")
    subprocess.run([sys.executable, str(ROOT / "scripts/fit_calib.py")])


if __name__ == "__main__":
    main(*sys.argv[1:])
