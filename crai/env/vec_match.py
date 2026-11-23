"""多局并行离线对战：一个进程同时跑 N 局，每 5 tick 把所有局要决策的 FirstLight 观测拼成一批在 GPU 上推理。

为什么这样做（2026-11-21 剖析一局 74 秒：模型推理 69%、引擎往返 22%）：
  - 逐局推理时 GPU 每次只算 1 个局面；批量后 GPU 算 N 个局面的耗时与 1 个相近；
  - 每局一个引擎连接（代理库多客户端，各自一局），N 局的 STEP 请求用线程并发发出，往返延迟互相重叠；
  - 多进程方案不行：CPU 推理慢，多进程共用 GPU 在 Windows 上频繁切换上下文。

用法：
    run_many(make_game, n_parallel, on_done)
      make_game(env) -> MatchState | None   用给定的引擎连接新开一局（返回 None 表示不再开新局）
      on_done(ms, result | None)            一局结束（result=None 表示这局因引擎崩溃作废）
"""

from __future__ import annotations

import time
from concurrent.futures import ThreadPoolExecutor

from crai.env.fl_agent import batch_decide
from crai.env.match import MatchState
from crai.env.native_env import NativeEnv


def _reconnect_all(envs: list) -> list:
    """引擎（游戏进程）崩溃：所有连接都断了。确认游戏重新跑起来后全部重连。"""
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "scripts"))
    from headless_resim import ensure_game
    for e in envs:
        try:
            e.close()
        except Exception:  # noqa: BLE001
            pass
    time.sleep(5)
    ensure_game()
    return [NativeEnv(wait=300) for _ in envs]


def run_many(make_game, n_parallel: int, on_done, log=print) -> dict:
    envs = [NativeEnv(wait=300) for _ in range(n_parallel)]
    slots: list[MatchState | None] = [None] * n_parallel
    stats = {"games": 0, "crashes": 0, "rounds": 0, "infer_s": 0.0, "step_s": 0.0, "pre_s": 0.0}
    exhausted = False
    pool = ThreadPoolExecutor(max_workers=n_parallel)
    try:
        while True:
            # 空槽位开新局
            for k in range(n_parallel):
                if slots[k] is None and not exhausted:
                    ms = make_game(envs[k])
                    if ms is None:
                        exhausted = True
                        continue
                    ms.start()
                    slots[k] = ms
            active = [k for k in range(n_parallel) if slots[k] is not None]
            if not active:
                break
            try:
                # 1) 各局整理局面，收集要推理的观测
                t = time.time()
                need = []
                for k in active:
                    try:
                        got = slots[k].pre()
                    except (ConnectionError, OSError):
                        raise
                    except Exception as e:  # noqa: BLE001 —— 单局出错（数据不被支持等）只作废这一局
                        log(f"  第 {k} 槽这局出错作废：{type(e).__name__}: {e}")
                        stats.setdefault("errors", 0)
                        stats["errors"] += 1
                        on_done(slots[k], None)
                        slots[k] = None
                        continue
                    for owner, ag, obs in got:
                        need.append((k, owner, ag, obs))
                active = [k for k in active if slots[k] is not None]
                stats["pre_s"] += time.time() - t
                # 2) 批量推理，3) 逐局注入指令
                t = time.time()
                if need:
                    decisions = batch_decide([(ag, obs) for _, _, ag, obs in need])
                    for (k, owner, ag, _), d in zip(need, decisions):
                        slots[k].post(owner, ag, d)
                stats["infer_s"] += time.time() - t
                # 4) 所有局并发推进 5 tick
                t = time.time()
                errs = list(pool.map(lambda k: _safe_step(slots[k]), active))
                stats["step_s"] += time.time() - t
            except (ConnectionError, OSError) as e:      # 注入指令时连接断开
                errs = [str(e) or type(e).__name__]
            stats["rounds"] += 1
            if any(errs):
                # 一个连接断开说明游戏进程崩溃了：这一批的局全部作废，重连
                stats["crashes"] += 1
                log(f"  引擎断开（{next(e for e in errs if e)}），作废 {len(active)} 局，重连")
                for k in active:
                    on_done(slots[k], None)
                    slots[k] = None
                envs = _reconnect_all(envs)
                continue
            for k in active:
                if slots[k].done:
                    on_done(slots[k], slots[k].result())
                    stats["games"] += 1
                    slots[k] = None
    finally:
        pool.shutdown(wait=False)
        for e in envs:
            try:
                e.close()
            except Exception:  # noqa: BLE001
                pass
    return stats


def _safe_step(ms: MatchState):
    try:
        ms.step()
        return None
    except (ConnectionError, OSError) as e:
        return str(e) or type(e).__name__
