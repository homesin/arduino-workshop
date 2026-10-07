/*
 * 「還沒填 LINE token」的情境，必須在乾淨的環境變數下跑，
 * 所以由 selftest.js 另外開一個行程執行這支，結果用 JSON 印回去。
 */
process.env.LINE_CHANNEL_ACCESS_TOKEN = '';
process.env.ALERT_COOLDOWN_SEC = '5';

const { createLineNotifier } = await import('../src/line.js');
const line = createLineNotifier({ log: () => {} });

const first = await line.alert('pir', '第一次');
const second = await line.alert('pir', '緊接著第二次');

console.log(JSON.stringify({
  first: first.skipped,
  second: second.skipped,
  sentToday: line.stats.sentToday,
}));
