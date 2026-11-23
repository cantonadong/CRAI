"""实战：FirstLight 野猪模型打训练营或天梯（点击下牌）。

流程：开局 → 实时读取器定位当前对局的世界对象（roster）→ 反复向游戏内代理库请求实时帧（LIVE，含手牌 pd、单位种类）
→ FLAgent 决策 → 出牌指令换算成手牌槽位和"己方在下方"坐标 → 拖拽下牌。
用法：python -u scripts/run_fl_live.py [局数 1] [training|ladder] [模型 hog2]
"""
import json
import sys
import time
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT)); sys.path.insert(0, str(ROOT / "scripts"))
import start_training  # noqa: E402
from crai.bot.rules import Hog26Bot  # noqa: E402
from crai.client.control import Screen, deploy, deselect, press_ability  # noqa: E402
from crai.data import cards  # noqa: E402
from crai.env.fl_agent import FLAgent  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402
from crai.reader.stream import LiveReader  # noqa: E402
from run_bot import on_result_screen, tower_summary  # noqa: E402

# 用户卡组形态：进化加农炮、进化骷髅兵、英雄火枪手（configs/memory: user-deck）
OWN_FORMS = {cards.card_id("cannon"): 1, cards.card_id("skeletons"): 1, cards.card_id("musketeer"): 2}


def own_side_of(frame) -> int:
    return frame.players[frame.own_index()].side


def play_one(scr, env, log, model, make_agent=None, hold=None):
    """打一局。make_agent(own, forms) 可替换智能体（天梯学习用 PPOLearner）；hold 返回本局使用的智能体（hold["agent"]）
    和最后一帧原生帧（hold["raw"]）。"""
    hold = hold if hold is not None else {}
    make_agent = make_agent or (lambda own, forms: FLAgent(own, forms, model=model))
    live = LiveReader(period_ms=50, rescan=3)
    ability_wait = None        # (按下时的技能次数, 时间)
    t0, n, last_check, plays, last_reader = time.time(), -1, time.time(), 0, None
    agent = None
    acct = 0
    pending = None             # (下牌时的出牌序号, 时间, tick)：只用来测出牌延迟
    inflight = {}              # 槽位 -> (卡组序号, 下牌时出牌序号, 时间)：防止同一张牌重复点
    errors = streak = rebuilds = 0
    fallback = None            # FirstLight 持续失败时接管的规则 bot
    hb = {"frames": 0, "skip_reader": 0, "raw_none": 0, "blocked": 0, "decide": 0, "cmds": 0}
    last_hb = time.time()
    try:
        while time.time() - t0 < 420:
            if time.time() - last_hb > 20:             # 心跳：排查"中途不出牌"
                last_hb = time.time()
                print(f"  [{time.time() - t0:6.1f}s 心跳] {hb} pending={pending is not None} ability_wait={ability_wait is not None}"
                      f" 模型={getattr(agent, 'diag', None)}")
                if raw_last := hold.get("raw"):
                    me_ = next((p for p in raw_last["players"] if p["side"] == own_side_of(last_reader)), None) if last_reader else None
                    if me_:
                        print(f"           圣水 {me_.get('elixir')} 手牌 {me_.get('hand')} pd {me_.get('pd')}")
                log.flush()
            n, f = live.latest(newer_than=n, timeout=10)
            hb["frames"] += 1
            if not (f.battle and f.players and f.roster):
                hb["skip_reader"] += 1
                continue
            last_reader = f
            own = own_side_of(f)
            if fallback is not None:                    # FirstLight 持续失败：规则 bot 接管
                act = fallback.decide(f)
                if fallback.need_deselect:
                    deselect(); fallback.need_deselect = False
                if act:
                    deploy(scr, act.slot, act.x, act.y)
                    plays += 1
                    print(f"  [{time.time() - t0:6.1f}s 规则] {act.reason}")
                if time.time() - last_check > 3:
                    last_check = time.time()
                    if time.time() - t0 > 30 and on_result_screen():
                        break
                continue
            t_live = time.time()
            raw = env.live(f.roster)
            t_dec = time.time()
            if raw is None:
                hb["raw_none"] += 1
                continue
            if agent is None:
                me = next(p for p in raw["players"] if p["side"] == own)
                acct = me["acct"]
                forms = {own: OWN_FORMS, 1 - own: {}}
                agent = make_agent(own, forms)
                agent.stall_guard = True               # 圣水满且 15 秒没出牌时强制出一张（见 FLAgent.prepare）
                hold["agent"] = agent
                log.write(json.dumps({"type": "start", "own_side": own, "tick": raw["tick"]}) + "\n")
                print(f"  开局：己方 side{own}，tick {raw['tick']}")
            # 下牌后到手牌变化之间有延迟：等出牌序号增加（或 1.5 秒超时）前不让模型决策，避免同一张牌下两次，
            # 也避免被丢弃的动作写进模型的动作历史 / 在途牌
            hold["raw"] = raw
            me0 = next(p for p in raw["players"] if p["side"] == own)
            if pending is not None and (me0["seq"] > pending[0] or time.time() - pending[1] > 1.5):
                if me0["seq"] > pending[0] and len(pending) > 2:     # 出牌延迟：模型决策 tick → 游戏里出牌序号增加
                    lat = hold.setdefault("latencies", [])
                    lat.append(int(raw["tick"]) - pending[2])
                    hb["lat_avg_ticks"] = round(sum(lat) / len(lat), 1)
                    hb["lat_max_ticks"] = max(lat)
                pending = None
            chg = next((r["chg"] for r in me0.get("abil", []) if r.get("mc")), None)
            if ability_wait is not None:
                if chg is not None and chg < ability_wait[0]:
                    log.write(json.dumps({"type": "ability_confirmed", "tick": raw["tick"],
                                          "ms": round((time.time() - ability_wait[1]) * 1000)}) + "\n")
                    print(f"  [tick {raw['tick']:5d}] 技能已释放")
                    ability_wait = None
                    pending = None
                elif time.time() - ability_wait[1] > 1.5:
                    log.write(json.dumps({"type": "ability_unconfirmed", "tick": raw["tick"]}) + "\n")
                    print(f"  [tick {raw['tick']:5d}] 技能按下后 1.5 秒没生效")
                    ability_wait = None
            # 出牌后等确认期间照常决策（2026-11-22 改）：实测确认要约 25 tick（1.25 秒），以前整段不决策，
            # 模型每下一张牌就"失聪"一秒多，和离线训练（每 5 tick 决策一次）不一致，表现为解牌偏晚。
            # 同一张牌不会下两次：FirstLight 的在途记录（_note_pending）会屏蔽那个槽位并预留圣水；下面再按槽位兜底一次。
            # 只有英雄技能等确认期间不决策。
            blocked = ability_wait is not None
            hb["blocked" if blocked else "decide"] += 1
            try:
                cmds = agent.decide_raw(raw, acct, allow=not blocked)
                hb["cmds"] += len(cmds)
                t_done = time.time()
            except Exception as e:  # noqa: BLE001 —— 单帧出错不能中断整局
                errors += 1
                streak += 1
                log.write(json.dumps({"type": "error", "tick": raw["tick"], "error": f"{type(e).__name__}: {e}"},
                                     ensure_ascii=False) + "\n")
                if errors <= 5:
                    print(f"  决策出错（跳过这一帧）：{type(e).__name__}: {e}")
                if streak >= 10:                       # 连续出错：适配器状态已坏，从当前帧重建智能体继续打
                    rebuilds += 1
                    if rebuilds >= 3:                  # 重建也救不回来（例如数据本身不被支持）：规则 bot 接管
                        print(f"  FirstLight 重建 {rebuilds} 次仍失败，改由规则 bot 接管（tick {raw['tick']}）")
                        fallback = Hog26Bot()
                    else:
                        print(f"  连续 {streak} 帧出错，重建 FirstLight 智能体（tick {raw['tick']}）")
                        agent = make_agent(own, {own: OWN_FORMS, 1 - own: {}})
                        hold["agent"] = agent
                    streak = 0
                continue
            streak = 0
            me = me0
            king = next((e for e in raw["entities"]
                         if e["card"] == -1 and e["x"] == 9000 and e["y"] in (3000, 29000) and e["side"] == own), None)
            flip = king is not None and king["y"] < 16000
            # 一个决策点可能有两条出牌（FirstLight 的"第二个动作"，例如野猪+火枪手同时压上）：全部执行。
            # 出掉的牌由下一张补到同一个槽位，其他槽位编号不变，所以第二条仍按这一帧的手牌换算槽位。
            used_slots = set()
            for cmd in cmds:
                c = cmd["c"]
                if cmd.get("ct") == 189:                   # 英雄技能：点技能按钮，等次数下降确认
                    press_ability()
                    ability_wait = (chg if chg is not None else 1, time.time())
                    log.write(json.dumps({"type": "ability", "tick": raw["tick"], "cgid": c["cgid"]}) + "\n")
                    print(f"  [tick {raw['tick']:5d}] 按技能按钮")
                    continue
                d = ((c["sel"]["pd"] >> 22) & 0x3F) - 1
                if d not in me["hand"]:
                    continue
                slot = me["hand"].index(d)
                if slot in used_slots:
                    continue
                fl = inflight.get(slot)                    # 兜底：这张牌刚点过、游戏里还没执行，不重复下
                if fl and fl[0] == d and fl[1] == me["seq"] and time.time() - fl[2] < 1.5:
                    hb["skip_inflight"] = hb.get("skip_inflight", 0) + 1
                    continue
                used_slots.add(slot)
                x, y = c["px"], c["py"]
                if flip:                                   # 引擎坐标 → 己方在下方的视角（点击用）
                    x, y = 18000 - x, 32000 - y
                t_tap = time.time()
                deploy(scr, slot, x, y)
                t_tapped = time.time()
                plays += 1
                inflight[slot] = (d, me["seq"], time.time())
                if pending is None:                        # 只用来测出牌延迟
                    pending = (me["seq"], time.time(), int(raw["tick"]))
                name = cards.ID2SLUG.get(c["sel"]["os"], c["sel"]["os"])
                # 延迟拆分（毫秒）：帧在读取器里的读取耗时 / 帧到达后等多久才开始处理 / LIVE 往返 / 模型 / 点击
                timing = {"read": round(getattr(f, "read_ms", 0) or 0), "queue": round((t_live - getattr(f, "arrived", t_live)) * 1000),
                          "live": round((t_dec - t_live) * 1000), "model": round((t_done - t_dec) * 1000),
                          "tap": round((t_tapped - t_tap) * 1000), "bin": int(c["t2"]) - int(c["t"]) - 20}
                tim = hold.setdefault("timing", [])
                tim.append(timing)
                rec = {"t": round(time.time() - t0, 2), "tick": raw["tick"], "card": name, "x": x, "y": y, "slot": slot,
                       "n_cmds": len(cmds), "ms": timing}
                log.write(json.dumps({"type": "play", **rec}, ensure_ascii=False) + "\n")
                print(f"  [{rec['t']:6.1f}s tick {raw['tick']:5d}] {name} -> ({x},{y}) 槽 {slot}"
                      + ("（双动作）" if len(cmds) > 1 else ""))
            if time.time() - last_check > 3:
                last_check = time.time()
                if time.time() - t0 > 30 and on_result_screen():
                    break
    finally:
        live.close()
        log.flush()
    res = {"duration": round(time.time() - t0, 1), "plays": plays, "errors": errors,
           "fallback": fallback is not None}
    if hold.get("timing"):                              # 本局出牌延迟拆分的平均值（毫秒；bin 是模型自己选的延后刻数）
        tim = hold["timing"]
        res["timing_avg"] = {k: round(sum(t[k] for t in tim) / len(tim), 1) for k in tim[0]}
    if hold.get("latencies"):
        res["lat_avg_ticks"] = round(sum(hold["latencies"]) / len(hold["latencies"]), 1)
    if last_reader is not None:
        res.update(tower_summary(last_reader))
    if "own_towers" in res:
        res["result"] = "win" if res["enemy_towers"] < res["own_towers"] else (
            "lose" if res["enemy_towers"] > res["own_towers"] else "draw")
    return res


def main(n_games="1", mode="training", model="hog2"):
    sys.stdout.reconfigure(line_buffering=True)
    out_dir = ROOT / "data" / "bot_runs"
    out_dir.mkdir(parents=True, exist_ok=True)
    path = out_dir / f"fl_{datetime.now():%Y%m%d_%H%M%S}.jsonl"
    scr = Screen()
    env = NativeEnv()
    summary = []
    with path.open("w", encoding="utf-8") as log:
        for g in range(int(n_games)):
            if start_training.main(mode) != 0:
                print("开局失败")
                break
            print(f"== 第 {g + 1} 局开始（{mode}，模型 {model}）")
            res = play_one(scr, env, log, model)
            res["game"] = g + 1
            log.write(json.dumps({"type": "result", **res}, ensure_ascii=False) + "\n")
            log.flush()
            summary.append(res)
            print(f"== 第 {g + 1} 局结束：{res}")
            time.sleep(2)
    env.close()
    print(f"共 {len(summary)} 局，胜 {sum(r.get('result') == 'win' for r in summary)}；日志 {path}")


if __name__ == "__main__":
    main(*sys.argv[1:])
