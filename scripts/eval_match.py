"""在离线引擎里让两个 AI 对打 N 局（交替换边、每局换随机种子），统计胜负。双方都用野猪 2.6。

用法：python -u scripts/eval_match.py <A> <B> [局数 10]
AI：rules（规则 bot）、v2（模仿学习模型 models/hog26_v2.pt）、idle（从不出牌，用来做基本检查）
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from crai.env.match import play_match  # noqa: E402
from crai.env.native_env import NativeEnv  # noqa: E402


class Idle:
    def decide(self, frame, now=None):
        return None


def make(name, side=0):
    if name.startswith("fl"):                       # fl / fl-hog1 / fl-general …（FirstLight 模型，默认野猪专精 2）
        from crai.data import cards
        from crai.env.fl_agent import FLAgent
        from crai.env.match import HOG26_SPEC
        forms = {s: {cards.card_id(k): v for k, v in HOG26_SPEC.forms.items()} for s in (0, 1)}
        return FLAgent(side, forms, model=name.split("-", 1)[1] if "-" in name else "hog2")
    if name == "rules":
        from crai.bot.rules import Hog26Bot
        return Hog26Bot()
    if name == "v2":
        from crai.bot.v2_bot import V2Bot
        return V2Bot()
    if name == "idle":
        return Idle()
    raise ValueError(name)


def main(a, b, n="10"):
    env = NativeEnv()
    score = {a + "(A)": 0, b + "(B)": 0, "平": 0}
    crowns_a = crowns_b = 0
    for g in range(int(n)):
        a_side = g % 2
        agents = {a_side: make(a, a_side), 1 - a_side: make(b, 1 - a_side)}
        r = play_match(agents[0], agents[1], seed=1000 + g, env=env)
        ca, cb = r.crowns[a_side], r.crowns[1 - a_side]
        crowns_a += ca; crowns_b += cb
        key = a + "(A)" if ca > cb else (b + "(B)" if cb > ca else "平")
        score[key] += 1
        print(f"第 {g + 1:2d} 局 A 在 side{a_side}：{a} {ca} : {cb} {b}  tick {r.ticks}  "
              f"出牌 A {r.plays[a_side]}/被拒 {r.rejected[a_side]}  B {r.plays[1 - a_side]}/被拒 {r.rejected[1 - a_side]}  "
              f"{r.seconds}s", flush=True)
    env.close()
    print(f"\n结果：{score}，总皇冠 A {crowns_a} : {crowns_b} B")


if __name__ == "__main__":
    main(*sys.argv[1:])
