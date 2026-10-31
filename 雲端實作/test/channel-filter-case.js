/*
 * NOTIFY_CHANNELS 的過濾行為，必須在乾淨的環境變數下跑，
 * 所以由 selftest.js 另外開一個行程執行這支，結果用 JSON 印回去。
 *
 * 情境：.env 裡五家的憑證都填了，但只想測 ntfy 一家。
 */
const { createNotifier } = await import('../src/notify.js');
const notifier = createNotifier({ log: () => {} });

console.log(JSON.stringify({
  channels: notifier.channels.map((c) => c.name),
}));
