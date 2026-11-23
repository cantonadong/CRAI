// ==UserScript==
// @name         RoyaleAPI Replay URL 抓取
// @namespace    https://royaleapi.com/
// @version      1.0
// @description  在 RoyaleAPI 对战记录页点击 Replay 按钮时，显示加载回放数据的请求 URL，可一键复制或打开
// @match        https://royaleapi.com/*
// @grant        none
// @run-at       document-start
// ==/UserScript==

(function () {
  'use strict';

  const REPLAY_PATH = '/data/replay';

  // ---------- 面板 ----------
  let panel, urlBox, srcLabel;

  function ensurePanel() {
    if (panel) return;
    panel = document.createElement('div');
    panel.style.cssText = [
      'position:fixed', 'right:16px', 'bottom:16px', 'z-index:2147483647',
      'width:520px', 'max-width:calc(100vw - 32px)', 'background:#1e1f24', 'color:#eee',
      'font:12px/1.5 system-ui,sans-serif', 'border-radius:8px', 'padding:10px 12px',
      'box-shadow:0 4px 18px rgba(0,0,0,.4)', 'display:none'
    ].join(';');

    const head = document.createElement('div');
    head.style.cssText = 'display:flex;justify-content:space-between;align-items:center;margin-bottom:6px';
    srcLabel = document.createElement('b');
    const close = document.createElement('span');
    close.textContent = '✕';
    close.style.cssText = 'cursor:pointer;opacity:.7';
    close.onclick = () => (panel.style.display = 'none');
    head.append(srcLabel, close);

    urlBox = document.createElement('textarea');
    urlBox.readOnly = true;
    urlBox.style.cssText = 'width:100%;height:70px;box-sizing:border-box;background:#111;color:#9fe39f;border:1px solid #333;border-radius:4px;padding:6px;font:11px monospace;resize:vertical';

    const btns = document.createElement('div');
    btns.style.cssText = 'display:flex;gap:8px;margin-top:6px';
    const mkBtn = (txt, fn) => {
      const b = document.createElement('button');
      b.textContent = txt;
      b.style.cssText = 'background:#f2711c;color:#fff;border:0;border-radius:4px;padding:4px 10px;cursor:pointer';
      b.onclick = fn;
      return b;
    };
    const copyBtn = mkBtn('复制', async () => {
      try { await navigator.clipboard.writeText(urlBox.value); }
      catch { urlBox.select(); document.execCommand('copy'); }
      copyBtn.textContent = '已复制';
      setTimeout(() => (copyBtn.textContent = '复制'), 1200);
    });
    const openBtn = mkBtn('新标签打开', () => window.open(urlBox.value, '_blank'));
    btns.append(copyBtn, openBtn);

    panel.append(head, urlBox, btns);
    document.body.appendChild(panel);
  }

  function show(url, source) {
    if (!document.body) return;
    ensurePanel();
    urlBox.value = url;
    srcLabel.textContent = source === 'network'
      ? 'Replay 数据请求（实际网络请求）'
      : 'Replay 数据 URL（按按钮参数拼出，等待网络请求…）';
    panel.style.display = 'block';
  }

  const isReplay = (u) => {
    try { return new URL(u, location.href).pathname === REPLAY_PATH; } catch { return false; }
  };
  const abs = (u) => new URL(u, location.href).href;

  // ---------- 1. 拦截真实请求（XHR + fetch） ----------
  const origOpen = XMLHttpRequest.prototype.open;
  XMLHttpRequest.prototype.open = function (method, url) {
    if (isReplay(url)) {
      const full = abs(url);
      this.addEventListener('load', () => show(full, 'network'));
    }
    return origOpen.apply(this, arguments);
  };

  const origFetch = window.fetch;
  window.fetch = function (input) {
    const url = typeof input === 'string' ? input : input && input.url;
    if (url && isReplay(url)) {
      const full = abs(url);
      return origFetch.apply(this, arguments).then((r) => { show(full, 'network'); return r; });
    }
    return origFetch.apply(this, arguments);
  };

  // ---------- 2. 点击时按按钮的 data-* 参数立即拼出 URL ----------
  // （回放已加载过、再次点击不会重新请求时，也能拿到 URL）
  document.addEventListener('click', (e) => {
    const btn = e.target.closest && e.target.closest('.replay_button');
    if (!btn) return;
    const d = btn.dataset;
    if (!d.replay) return;
    const p = new URLSearchParams({
      tag: d.replay,
      team_tags: d.teamTags || '',
      opponent_tags: d.opponentTags || '',
      team_crowns: d.teamCrowns || '',
      opponent_crowns: d.opponentCrowns || '',
      referrer_path: location.origin + location.pathname,
    });
    show(location.origin + REPLAY_PATH + '?' + p.toString(), 'built');
  }, true);
})();