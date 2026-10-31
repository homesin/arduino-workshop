/*
 * 手動發一則測試警報，用來確認某一家通知服務到底有沒有接通。
 *
 *   node test/send-test-alert.js            送給 .env 裡所有已設定的通道
 *   node test/send-test-alert.js ntfy       只送 ntfy（不動用其他家的額度）
 *   node test/send-test-alert.js line
 *
 * 這支會真的送出訊息、真的消耗額度，跟自我測試（npm run selftest）不一樣。
 */
const only = process.argv[2];
if (only) process.env.NOTIFY_CHANNELS = only; // 必須在 import config 之前設

const { createNotifier } = await import('../src/notify.js');

const stamp = new Date().toLocaleString('zh-TW', { hour12: false });
const log = (msg) => console.log(`  ${msg}`);

const notifier = createNotifier({ log });

if (notifier.channels.length === 0) {
  console.log('');
  console.log('  沒有任何通道被啟用。檢查 .env 裡的憑證有沒有填。');
  console.log('');
  process.exit(1);
}

console.log('');
console.log(`  要送往：${notifier.channels.map((c) => c.name).join('、')}`);
console.log('');

const result = await notifier.alert('test', `測試警報 · 校園環境監測站\n這是一則手動發出的測試訊息\n時間 ${stamp}`);

console.log('');
for (const r of result.results) {
  if (r.ok) console.log(`  ✔ ${r.channel} 送出成功`);
  else if (r.skipped === 'no-token') console.log(`  — ${r.channel} 未設定憑證（只做預演）`);
  else if (r.skipped === 'daily-limit') console.log(`  ✘ ${r.channel} 已達今日上限`);
  else if (r.skipped === 'cooldown') console.log(`  — ${r.channel} 冷卻中，還要等 ${r.waitSec} 秒`);
  else console.log(`  ✘ ${r.channel} 失敗：${r.status ?? ''} ${r.error ?? ''}`);
}
console.log('');
process.exit(result.anyOk ? 0 : 1);
