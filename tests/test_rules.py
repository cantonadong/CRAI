from crai.bot.rules import CANNON, HOG, ICE_GOLEM, LOG, MUSKETEER, SKELETONS, FIREBALL, ICE_SPIRIT, Hog26Bot
from crai.reader.stream import Entity, Frame, Player

DECK = [SKELETONS, MUSKETEER, CANNON, HOG, FIREBALL, LOG, ICE_SPIRIT, ICE_GOLEM]


def towers():
    return [Entity("k0", 13, 0, 9000, 3000, -1, 13, 4000, 4000),
            Entity("k1", 13, 1, 9000, 29000, -1, 16, 7000, 7000),
            Entity("p0l", 13, 0, 3500, 6500, -1, 13, 3000, 3000),
            Entity("p0r", 13, 0, 14500, 6500, -1, 13, 2000, 3000)]


def frame(hand_idx, elixir, seq, extra=()):
    me = Player(1, elixir, seq, (7000, 7000), hand_idx, [4, 5, 6, 7], DECK)
    opp = Player(0, 5.0, 0, (4000, 4000), [0, 1, 2, 3], [4, 5, 6, 7], DECK)
    return Frame(t=0, battle=True, players=[opp, me], entities=towers() + list(extra))


def test_hog_attacks_weaker_lane_and_no_repeat():
    bot = Hog26Bot()
    f = frame([3, 0, 1, 2], 8.0, seq=5)  # 手牌: 野猪 骷髅 火枪 加农
    a = bot.decide(f, now=100.0)
    assert a.card == HOG and a.slot == 0 and a.x == 14500  # 右塔血更少
    # 序号未变：即使过了冷却也不重复下
    assert bot.decide(f, now=101.0) is None
    # 序号 +1 后恢复决策
    f2 = frame([4, 0, 1, 2], 4.0, seq=6)
    bot.decide(f2, now=101.2)


def test_enemy_spell_is_not_a_threat():
    bot = Hog26Bot()
    graveyard = Entity("g", 15, 0, 3000, 27000, 28000010, 13, -1, -1)
    f = frame([7, 0, 1, 2], 5.0, seq=1, extra=[graveyard])
    a = bot.decide(f, now=10.0)
    assert a is None or "拦截" not in a.reason


def test_defends_crossing_troop():
    bot = Hog26Bot()
    giant = Entity("x", 15, 0, 3500, 20000, 26000003, 13, 3000, 3000)
    f = frame([7, 0, 1, 3], 5.0, seq=1, extra=[giant])  # 冰人 骷髅 火枪 野猪
    a = bot.decide(f, now=10.0)
    assert a is not None and a.card in (ICE_GOLEM, CANNON, MUSKETEER, SKELETONS)
