// ==UserScript==
// @name         RoyaleAPI 回放自动下载（批量 Top200）
// @namespace    https://royaleapi.com/
// @version      4.0
// @description  在 RoyaleAPI 玩家对战页自动保存所有对局 replay（replay0001.json 起自增，跳过已下载，识别限流）；同时保存该局的对战行 HTML（replayNNNN.row.html，含双方 8 张卡、进化/英雄形态、等级、塔兵），供 CRAI 精确重演。排行榜页可一键批量：依次打开前 N 名玩家的对战页自动下载。
// @match        https://royaleapi.com/player/*
// @match        https://royaleapi.com/players/*
// @grant        none
// @run-at       document-idle
// ==/UserScript==
// 基于用户的 api_greak.txt v3.2。新增：
//  1. 保存对战行 HTML（卡组形态/等级/塔兵，RoyaleAPI 回放 JSON 里没有这些）。
//  2. 批量模式：排行榜页点「批量下载前 N 名」→ 玩家队列存在 localStorage → 逐个打开 /player/<tag>/battles，
//     该页全部对局下载完后停 15 秒自动跳到下一位。随时可点「停止批量」。

(function () {
  'use strict';

  const REPLAY_PATH = '/data/replay';
  const PREFIX = 'replay';
  const PAD = 4;
  const MANIFEST = '_manifest.json';
  let delayMs = 5000;
  const MAX_DELAY_MS = 30000;
  const BACKOFF_START_MS = 3 * 60000;
  const BACKOFF_MAX_MS = 30 * 60000;
  const MAX_RETRY = 3;
  const NEXT_PLAYER_PAUSE_MS = 15000;
  const BATCH_KEY = 'crai-batch';     // localStorage：{players:[tag...], pos:n, on:bool}
  let backoffMs = BACKOFF_START_MS;
  const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
  const AUTO_RUN = true;

  const loadBatch = () => { try { return JSON.parse(localStorage.getItem(BATCH_KEY)) || null; } catch { return null; } };
  const saveBatch = (b) => { try { localStorage.setItem(BATCH_KEY, JSON.stringify(b)); } catch {} };
  const isBattlesPage = /^\/player\/[^/]+\/battles/.test(location.pathname);
  const isLeaderboard = location.pathname.startsWith('/players/');

  // ---------- IndexedDB：保存文件夹句柄 ----------
  const idb = (mode, fn) => new Promise((res, rej) => {
    const open = indexedDB.open('cr-replay-dl', 1);
    open.onupgradeneeded = () => open.result.createObjectStore('kv');
    open.onerror = () => rej(open.error);
    open.onsuccess = () => {
      const tx = open.result.transaction('kv', mode);
      const req = fn(tx.objectStore('kv'));
      tx.oncomplete = () => res(req && req.result);
      tx.onerror = () => rej(tx.error);
    };
  });
  // 批量 Top200 用单独的文件夹（D:\Dev\CRAI\replay_top200，通用数据），和手动下载的野猪 2.6 回放（D:\Dev\CRAI\replay）分开
  const dirKey = () => ((loadBatch() || {}).on ? 'dir_top200' : 'dir');
  const getHandle = () => idb('readonly', (s) => s.get(dirKey()));
  const setHandle = (h) => idb('readwrite', (s) => s.put(h, dirKey()));

  // ---------- 面板 ----------
  const panel = document.createElement('div');
  panel.style.cssText = 'position:fixed;right:16px;bottom:16px;z-index:2147483647;width:380px;max-width:calc(100vw - 32px);background:#1e1f24;color:#eee;font:12px/1.5 system-ui,sans-serif;border-radius:8px;padding:10px 12px;box-shadow:0 4px 18px rgba(0,0,0,.4)';
  const title = document.createElement('b');
  title.textContent = '回放下载 v4';
  const log = document.createElement('div');
  log.style.cssText = 'max-height:160px;overflow:auto;margin:6px 0;font:11px monospace;color:#9fe39f;white-space:pre-wrap';
  const btnRow = document.createElement('div');
  btnRow.style.cssText = 'display:flex;gap:8px;flex-wrap:wrap';
  const mkBtn = (txt, fn) => {
    const b = document.createElement('button');
    b.textContent = txt;
    b.style.cssText = 'background:#f2711c;color:#fff;border:0;border-radius:4px;padding:4px 10px;cursor:pointer';
    b.onclick = fn;
    btnRow.appendChild(b);
    return b;
  };
  const say = (msg) => { log.textContent += msg + '\n'; log.scrollTop = log.scrollHeight; };
  panel.append(title, log, btnRow);
  document.body.appendChild(panel);
  const statusLine = document.createElement('div');
  statusLine.style.cssText = 'color:#ccc;margin-top:4px';
  panel.insertBefore(statusLine, btnRow);

  // ---------- 排行榜页：建立批量队列 ----------
  if (isLeaderboard && !isBattlesPage) {
    const tags = [];
    document.querySelectorAll('a[href^="/player/"]').forEach((a) => {
      const m = a.getAttribute('href').match(/^\/player\/([0-9A-Z]+)/i);
      if (m && !tags.includes(m[1].toUpperCase())) tags.push(m[1].toUpperCase());
    });
    say(`本页找到 ${tags.length} 名玩家`);
    const b0 = loadBatch();
    if (b0 && b0.on) say(`批量进行中：第 ${b0.pos + 1}/${b0.players.length} 名`);
    mkBtn('批量下载前 200 名', () => {
      const n = parseInt(prompt('下载前多少名？（本页之外的名次请翻页后点「追加本页」）', '200'), 10) || 200;
      const players = tags.slice(0, n);
      saveBatch({ players, pos: 0, on: true });
      say(`已建立队列 ${players.length} 名，开始…`);
      location.href = `/player/${players[0]}/battles`;
    });
    mkBtn('追加本页', () => {
      const b = loadBatch() || { players: [], pos: 0, on: false };
      for (const t of tags) if (!b.players.includes(t)) b.players.push(t);
      saveBatch(b);
      say(`队列现有 ${b.players.length} 名`);
    });
    mkBtn('继续批量', () => {
      const b = loadBatch();
      if (!b || !b.players.length) return say('没有队列');
      b.on = true; saveBatch(b);
      location.href = `/player/${b.players[Math.min(b.pos, b.players.length - 1)]}/battles`;
    });
    mkBtn('停止批量', () => { const b = loadBatch(); if (b) { b.on = false; saveBatch(b); } say('已停止批量'); });
    mkBtn('隐藏', () => (panel.style.display = 'none'));
    return;
  }
  if (!isBattlesPage) { panel.remove(); return; }

  // ---------- 文件夹 ----------
  async function ensureDir(interactive) {
    let dir = await getHandle().catch(() => null);
    if (dir) {
      const opts = { mode: 'readwrite' };
      let perm = await dir.queryPermission(opts);
      if (perm !== 'granted' && interactive) perm = await dir.requestPermission(opts);
      if (perm === 'granted') return dir;
      if (!interactive) return null;
    }
    if (!interactive) return null;
    dir = await window.showDirectoryPicker({ id: 'cr-replay', mode: 'readwrite' });
    await setHandle(dir);
    return dir;
  }
  async function readManifest(dir) {
    try { const f = await (await dir.getFileHandle(MANIFEST)).getFile(); return JSON.parse(await f.text()); } catch { return {}; }
  }
  async function writeFile(dir, name, text) {
    const fh = await dir.getFileHandle(name, { create: true });
    const w = await fh.createWritable();
    await w.write(text);
    await w.close();
  }
  async function maxIndex(dir) {
    const re = new RegExp('^' + PREFIX + '(\\d+)\\.', 'i');
    let max = 0;
    for await (const name of dir.keys()) { const m = name.match(re); if (m) max = Math.max(max, parseInt(m[1], 10)); }
    return max;
  }
  function replayUrl(d) {
    const p = new URLSearchParams({
      tag: d.replay, team_tags: d.teamTags || '', opponent_tags: d.opponentTags || '',
      team_crowns: d.teamCrowns || '', opponent_crowns: d.opponentCrowns || '',
      referrer_path: location.origin + location.pathname,
    });
    return location.origin + REPLAY_PATH + '?' + p.toString();
  }
  function htmlText(html) {
    if (!html) return '';
    const div = document.createElement('div');
    div.innerHTML = html;
    return (div.textContent || '').replace(/\s+/g, ' ').trim();
  }
  // 对战行：从回放按钮往上找，找到同时包含两个卡组网格（id 以 deck_ 开头）的最小祖先
  function battleRowHtml(btn) {
    let el = btn;
    for (let i = 0; i < 15 && el; i++, el = el.parentElement) {
      if (el.querySelectorAll && el.querySelectorAll('[id^="deck_"]').length >= 2) return el.outerHTML;
    }
    return null;
  }

  // ---------- 主流程 ----------
  let dir = null, manifest = {}, idx = 0, ready = false, working = false, paused = false;
  const queue = [];
  const failed = [];
  const tries = {};
  const seen = new Set();
  let stat = { ok: 0, skip: 0, fail: 0 };
  const batch = loadBatch();
  const batchOn = !!(batch && batch.on);

  function enqueue(btn) {
    const tag = btn.dataset.replay;
    if (!tag || seen.has(tag)) return;
    seen.add(tag);
    if (manifest[tag]) { stat.skip++; status(); return; }
    queue.push({ dataset: { ...btn.dataset }, row: battleRowHtml(btn) });
    pump();
  }
  const io = new IntersectionObserver((entries) => {
    for (const e of entries) { if (!e.isIntersecting) continue; io.unobserve(e.target); enqueue(e.target); }
  }, { rootMargin: '300px 0px' });
  const watched = new WeakSet();
  function watchButtons(root = document) {
    root.querySelectorAll && root.querySelectorAll('.replay_button[data-replay]').forEach((b) => {
      if (watched.has(b)) return;
      watched.add(b);
      if (batchOn) enqueue(b); else io.observe(b);     // 批量模式：整页按钮直接入队，不用滚动
    });
  }
  new MutationObserver((muts) => {
    for (const m of muts) for (const n of m.addedNodes) {
      if (n.nodeType !== 1) continue;
      if (n.matches && n.matches('.replay_button[data-replay]')) { if (!watched.has(n)) { watched.add(n); batchOn ? enqueue(n) : io.observe(n); } }
      else watchButtons(n);
    }
  }).observe(document.body, { childList: true, subtree: true });

  function status() {
    const b = batchOn ? ` · 批量 ${batch.pos + 1}/${batch.players.length}` : '';
    statusLine.textContent = `${paused ? '[已暂停] ' : ''}已下载 ${stat.ok} · 跳过 ${stat.skip} · 失败 ${failed.length} · 排队 ${queue.length}${b}`;
  }
  async function countdown(ms, why) {
    say(`⏸ ${why}，暂停 ${Math.round(ms / 1000)} 秒；之后每场间隔 ${(delayMs / 1000).toFixed(1)} 秒`);
    const end = Date.now() + ms;
    while (Date.now() < end) {
      statusLine.textContent = `限流暂停中，${Math.ceil((end - Date.now()) / 1000)} 秒后继续 · 已下载 ${stat.ok} · 排队 ${queue.length}`;
      await sleep(1000);
    }
    status();
  }
  async function rateLimited(item, retryAfterSec, why) {
    queue.unshift(item);
    const wait = retryAfterSec > 0 ? retryAfterSec * 1000 : backoffMs;
    backoffMs = Math.min(backoffMs * 2, BACKOFF_MAX_MS);
    delayMs = Math.min(Math.round(delayMs * 1.5), MAX_DELAY_MS);
    await countdown(wait, why);
  }

  let advancing = false;
  async function nextPlayer() {
    if (advancing) return;
    const b = loadBatch();
    if (!b || !b.on) return;
    advancing = true;
    b.pos += 1;
    if (b.pos >= b.players.length) { b.on = false; saveBatch(b); say('✅ 批量完成'); return; }
    saveBatch(b);
    say(`本页完成，${NEXT_PLAYER_PAUSE_MS / 1000} 秒后打开第 ${b.pos + 1}/${b.players.length} 名`);
    await sleep(NEXT_PLAYER_PAUSE_MS);
    if ((loadBatch() || {}).on) location.href = `/player/${b.players[b.pos]}/battles`;
  }

  async function pump() {
    if (!ready || working || paused) return;
    working = true;
    while (queue.length && !paused) {
      const item = queue.shift();
      const d = item.dataset;
      status();
      if (manifest[d.replay]) { stat.skip++; continue; }
      try {
        const res = await fetch(replayUrl(d), { credentials: 'include' });
        if (res.status === 429) { await rateLimited(item, parseInt(res.headers.get('Retry-After'), 10), 'HTTP 429 限流'); continue; }
        if (!res.ok) throw new Error('HTTP ' + res.status);
        const text = await res.text();
        const json = JSON.parse(text);
        if (json.success === false) {
          const msg = htmlText(json.html);
          if (/rate limit/i.test(msg)) { await rateLimited(item, 0, '服务器提示 over your rate limit'); continue; }
          throw new Error('success=false ' + (msg ? '「' + msg.slice(0, 80) + '」' : ''));
        }
        const name = PREFIX + String(++idx).padStart(PAD, '0') + '.json';
        await writeFile(dir, name, text);
        if (item.row) await writeFile(dir, name.replace(/\.json$/, '.row.html'), item.row);
        manifest[d.replay] = {
          file: name, row: item.row ? name.replace(/\.json$/, '.row.html') : null,
          team_tags: d.teamTags, opponent_tags: d.opponentTags,
          team_crowns: d.teamCrowns, opponent_crowns: d.opponentCrowns,
          page: location.pathname, saved_at: new Date().toISOString(),
        };
        await writeFile(dir, MANIFEST, JSON.stringify(manifest, null, 2));
        stat.ok++;
        backoffMs = BACKOFF_START_MS;
        say(`✔ ${name}${item.row ? '+row' : ''}  (${d.replay} ${d.teamCrowns}-${d.opponentCrowns})`);
      } catch (e) {
        tries[d.replay] = (tries[d.replay] || 0) + 1;
        if (!failed.some((f) => f.dataset.replay === d.replay)) failed.push(item);
        say(`✘ ${d.replay}: ${e.message}（第 ${tries[d.replay]} 次）`);
      }
      status();
      await sleep(delayMs + Math.random() * 2000);
    }
    working = false;
    status();
    if (batchOn && !paused && queue.length === 0) nextPlayer();
  }

  async function start(interactive) {
    try {
      const h = await ensureDir(interactive);
      if (!h) {
        say(batchOn ? '批量模式：点「开始」，选择文件夹 D:\\Dev\\CRAI\\replay_top200（通用数据，与野猪 2.6 分开）'
                    : '点「开始」授权文件夹（野猪 2.6 回放选 D:\\Dev\\CRAI\\replay）');
        return;
      }
      dir = h;
      manifest = await readManifest(dir);
      idx = await maxIndex(dir);
      ready = true;
      say(`文件夹：${dir.name}，已有 ${Object.keys(manifest).length} 条记录，下一个编号 ${idx + 1}`);
      say(batchOn ? `批量模式：第 ${batch.pos + 1}/${batch.players.length} 名，整页自动下载` : '滚动页面即可，滚到的对局会自动下载');
      for (let i = queue.length - 1; i >= 0; i--) {
        if (manifest[queue[i].dataset.replay]) { queue.splice(i, 1); stat.skip++; }
      }
      status();
      pump();
      // 批量模式下整页都已下载过（全部跳过）时，直接跳下一位
      if (batchOn) setTimeout(() => { if (!working && queue.length === 0) nextPlayer(); }, 5000);
    } catch (e) {
      say('出错：' + e.message);
    }
  }

  mkBtn('开始', () => { if (!ready) start(true); });
  const pauseBtn = mkBtn('暂停', () => {
    paused = !paused;
    pauseBtn.textContent = paused ? '继续' : '暂停';
    say(paused ? '⏸ 已手动暂停（当前请求完成后停下）' : '▶ 继续');
    status();
    if (!paused) pump();
  });
  mkBtn('重试失败', () => {
    const list = failed.splice(0).filter((it) => (tries[it.dataset.replay] || 0) < MAX_RETRY && !manifest[it.dataset.replay]);
    if (!list.length) { say('没有可重试的失败项'); status(); return; }
    queue.push(...list);
    say(`↻ 重新排队 ${list.length} 场`);
    status();
    pump();
  });
  mkBtn('停止批量', () => { const b = loadBatch(); if (b) { b.on = false; saveBatch(b); } say('已停止批量（本页下完不再跳转）'); });
  mkBtn('更换文件夹', async () => {
    try {
      const h = await window.showDirectoryPicker({ id: 'cr-replay', mode: 'readwrite' });
      await setHandle(h);
      ready = false;
      say('已切换到：' + h.name);
      start(false);
    } catch {}
  });
  mkBtn('隐藏', () => (panel.style.display = 'none'));

  if (!window.showDirectoryPicker) {
    say('当前浏览器不支持 File System Access API，请用 Chrome / Edge');
  } else {
    watchButtons();
    status();
    if (AUTO_RUN) start(false);
  }
})();
