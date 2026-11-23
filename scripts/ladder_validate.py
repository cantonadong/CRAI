"""天梯验证：离线 PPO 每产出 N 次更新，就用最新检查点（argmax，和部署时一样）在天梯验证，记录胜负。
每个检查点：赢了继续打，输（或平）了就停，最多 MAX_GAMES=5 局。

  $env:CRAI_FL_ROOT="yesdelay"; $env:CRAI_INSTANCE="0"
  .venv\\Scripts\\python.exe -u scripts/ladder_validate.py <PPO 输出目录 models/ppo/run2> [每隔几次更新 10] [每次局数 6]

- 先验证起点（输出目录里最早的检查点，或 run 还没有检查点时等待）；
- 结果追加到 <输出目录>/ladder_eval.jsonl（每局一行）；汇总：python scripts/ladder_validate.py table <输出目录>；
- 跑在实例 0（天梯号）；离线训练在实例 1，互不干扰。
"""
import json
import os
import re
import sys
import time
from datetime import datetime
from pathlib import Path

os.environ.setdefault("CRAI_FL_ROOT", "yesdelay")
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))


MAX_GAMES = 5            # 用户定（2026-11-22）：每个检查点赢了继续、输（或平）了就停，最多 5 局


def checkpoints(run: Path) -> list[tuple[int, Path]]:
    out = []
    for p in run.glob("update-*.pt"):
        m = re.match(r"update-(\d+)\.pt", p.name)
        if m:
            out.append((int(m.group(1)), p))
    return sorted(out)


def table(run_dir: str):
    rows = [json.loads(x) for x in (ROOT / run_dir / "ladder_eval.jsonl").read_text(encoding="utf-8").splitlines() if x.strip()]
    by = {}
    for r in rows:
        by.setdefault(r["update"], []).append(r)
    print("更新   局数  胜  平  负  己方剩塔-敌方剩塔")
    for u, rs in sorted(by.items()):
        w = sum(r.get("result") == "win" for r in rs)
        d = sum(r.get("result") == "draw" for r in rs)
        diff = sum(r.get("own_towers", 0) - r.get("enemy_towers", 0) for r in rs)
        print(f"{u:5d}  {len(rs):4d}  {w:2d}  {d:2d}  {len(rs) - w - d:2d}  {diff:+d}")


def once(run_dir: str, ckpt: str, update: str, games: str = "6"):
    """由 ppo_train 调用：验证一个检查点后退出（先打 1 局，赢了打满）。期间 ppo_train 不在实例 0 上采集。"""
    sys.stdout.reconfigure(line_buffering=True)
    import start_training
    from crai.client.control import Screen
    from crai.env.native_env import NativeEnv
    from run_fl_live import play_one
    from native_runner.training.v4.policy_session import load_policy_v4
    run = ROOT / run_dir
    out = run / "ladder_eval.jsonl"
    u = int(update)
    games = str(min(int(games), MAX_GAMES))
    policy = load_policy_v4(ckpt, device="cpu")
    scr, env = Screen(), NativeEnv()
    live_log = (ROOT / "data/bot_runs" / f"ladder_validate_{datetime.now():%Y%m%d_%H%M%S}.jsonl").open("w", encoding="utf-8")
    print(f"{time.strftime('%H:%M:%S')} 验证 update {u}（{Path(ckpt).name}），赢了继续、输了停，最多 {games} 局")
    fails = 0
    with out.open("a", encoding="utf-8") as f:
        g = 0
        while g < int(games) and fails < 5:
            if start_training.main("ladder") != 0:
                fails += 1
                print("开局失败，30 秒后重试")
                time.sleep(30)
                continue
            res = play_one(scr, env, live_log, policy)
            g += 1
            rec = {"update": u, "game": g, "time": datetime.now().isoformat(timespec="seconds"), **res}
            f.write(json.dumps(rec, ensure_ascii=False) + "\n"); f.flush()
            print(f"  第 {g} 局 {res.get('result')} 己方剩塔 {res.get('own_towers')} 敌方剩塔 {res.get('enemy_towers')}"
                  f" 出牌 {res.get('plays')} 出错 {res.get('errors')}")
            time.sleep(2)
            if res.get("result") != "win":
                print(f"  第 {g} 局未赢，本检查点到此为止")
                break


def main(run_dir: str, every: str = "10", games: str = "6", *rest):
    if run_dir == "table":
        return table(every)
    if run_dir == "once":
        return once(every, games, *rest)
    sys.stdout.reconfigure(line_buffering=True)
    import start_training
    from crai.client.control import Screen
    from crai.env.native_env import NativeEnv
    from run_fl_live import play_one
    from native_runner.training.v4.policy_session import load_policy_v4
    run = ROOT / run_dir
    out = run / "ladder_eval.jsonl"
    done = set()
    if out.exists():
        done = {json.loads(x)["update"] for x in out.read_text(encoding="utf-8").splitlines() if x.strip()}
    scr = Screen()
    env = NativeEnv()
    live_log = (ROOT / "data/bot_runs" / f"ladder_validate_{datetime.now():%Y%m%d_%H%M%S}.jsonl").open("w", encoding="utf-8")
    while True:
        cks = checkpoints(run)
        if not cks:
            time.sleep(60)
            continue
        first = cks[0][0]
        due = [(u, p) for u, p in cks if u not in done and (u == first or (u - first) % int(every) == 0)]
        if not due:
            time.sleep(60)
            continue
        u, p = due[-1]                                        # 最新一个该验证的
        policy = load_policy_v4(str(p), device="cpu")
        print(f"{time.strftime('%H:%M:%S')} 验证 update {u}（{p.name}），先打 1 局，赢了再打满 {games} 局")
        with out.open("a", encoding="utf-8") as f:
            g = 0
            while g < int(games):
                if start_training.main("ladder") != 0:
                    print("开局失败，30 秒后重试")
                    time.sleep(30)
                    continue
                res = play_one(scr, env, live_log, policy)
                g += 1
                rec = {"update": u, "game": g, "time": datetime.now().isoformat(timespec="seconds"), **res}
                f.write(json.dumps(rec, ensure_ascii=False) + "\n"); f.flush()
                print(f"  第 {g} 局 {res.get('result')} 己方剩塔 {res.get('own_towers')} 敌方剩塔 {res.get('enemy_towers')}"
                      f" 出牌 {res.get('plays')} 出错 {res.get('errors')}")
                time.sleep(2)
                if g == 1 and res.get("result") != "win":     # 先打 1 局；赢了才继续打满，输/平就到此为止
                    print("  首局未赢，本检查点只打 1 局")
                    break
        done.add(u)


if __name__ == "__main__":
    main(*sys.argv[1:])
