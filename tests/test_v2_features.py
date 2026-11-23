"""v2 bot 的实战特征必须与训练数据集逐项一致：用离线重演的帧冒充实时帧比对。"""
import gzip
import json
from pathlib import Path

import numpy as np
import pytest

from crai.data import dataset_v2 as D

ROOT = Path(__file__).resolve().parents[1]


def _first_game():
    rows = [json.loads(l) for l in D.SUMMARY.read_text(encoding="utf-8").splitlines() if l.strip()]
    for r in rows:
        if "error" not in r and r.get("hog_owner") is not None and (D.OUT / f"{r['tag']}.npz").exists():
            return r
    return None


@pytest.mark.skipif(not D.SUMMARY.exists() or not (ROOT / "models/hog26_v2.pt").exists(), reason="需要重演数据和模型")
def test_live_features_match_dataset(monkeypatch):
    from crai.bot.v2_bot import V2Bot
    from crai.reader import stream
    r = _first_game()
    assert r is not None
    own = int(r["hog_owner"])
    data = np.load(D.OUT / f"{r['tag']}.npz")
    with gzip.open(D.SRC / f"{r['tag']}.jsonl.gz", "rt", encoding="utf-8") as f:
        frames = {fr["tick"]: fr for fr in map(json.loads, f)}
    bot = V2Bot()
    for row, tick in [(40, 200), (300, 1500), (500, 2500)]:
        fr = dict(frames[tick], t=0.0, battle=True)
        f = stream.parse_frame(json.dumps(fr))
        me_i = next(i for i, p in enumerate(f.players) if p.side == own)
        monkeypatch.setattr(type(f), "own_index", lambda self, i=me_i: i)
        s, c, e, _, _ = bot.features(f, tick)
        assert np.allclose(s.numpy()[0], data["scal"][row].astype(np.float32), atol=2e-3)
        assert (c.numpy()[0] == data["cards"][row]).all()
        # 单位顺序可以不同（模型对单位顺序不敏感），按内容排序后比较
        a = np.sort(e.numpy()[0].round(2), axis=0)
        b = np.sort(data["ents"][row].astype(np.float32).round(2), axis=0)
        assert np.allclose(a, b, atol=2e-2)
