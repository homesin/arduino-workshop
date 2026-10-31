import { config } from './config.js';

/**
 * 各家通知服務的共用外殼。
 *
 * 不管是 LINE、ntfy、Discord 還是 Telegram，「什麼時候該送」的規則都一樣：
 *   1. 同一種警報要有冷卻時間，不然感測器抖一下就洗版
 *   2. 每天有上限，避免測試時把額度燒光
 *   3. 沒設定憑證時要能「預演」，讓學員看到原本會送出什麼
 *
 * 真正每家不一樣的只有「怎麼發出這段文字」——那部分寫在 channels/ 底下，
 * 每個檔案只有十幾行。
 *
 * @param {object}   opts
 * @param {string}   opts.name          顯示用名稱，例如 'LINE'
 * @param {number}   opts.dailyLimit    每日上限，Infinity 表示不限
 * @param {boolean}  opts.configured    憑證是否已填好
 * @param {Function} opts.send          async (text) => Response，實際送出
 * @param {Function} [opts.explain]     (status) => string，解釋錯誤碼的提示
 * @param {boolean}  [opts.dryRun]      未設定時是否仍要預演（LINE 是，其他不是）
 * @param {Function} opts.log
 */
export function createChannelNotifier({ name, dailyLimit, configured, send, explain, dryRun = false, log }) {
  const lastSentAt = new Map(); // 每種警報各自計算冷卻
  let sentToday = 0;
  let dayStamp = new Date().toDateString();

  const rollDayIfNeeded = () => {
    const today = new Date().toDateString();
    if (today !== dayStamp) {
      dayStamp = today;
      sentToday = 0;
    }
  };

  return {
    name,
    configured,

    get stats() {
      rollDayIfNeeded();
      return { name, sentToday, dailyLimit };
    },

    async alert(kind, text) {
      rollDayIfNeeded();

      const since = Date.now() - (lastSentAt.get(kind) ?? 0);
      if (since < config.alertCooldownMs) {
        return { channel: name, skipped: 'cooldown', waitSec: Math.ceil((config.alertCooldownMs - since) / 1000) };
      }
      if (sentToday >= dailyLimit) {
        log(`已達今日 ${name} 上限 ${dailyLimit} 則，這則不送。`);
        return { channel: name, skipped: 'daily-limit' };
      }

      // 冷卻時間在這裡就開始算，不管有沒有真的送出去。
      // 這樣「預演」和「真的會送」的節奏完全一致，
      // 現場示範看到的頻率才不會跟正式上線時不一樣。
      lastSentAt.set(kind, Date.now());

      if (!configured) {
        if (dryRun) log(`[${name} 預演] 若已設定憑證，會送出：${text}`);
        return { channel: name, skipped: 'no-token' };
      }

      try {
        const res = await send(text);
        if (res.ok) {
          sentToday += 1;
          const quota = dailyLimit === Infinity ? `今日第 ${sentToday} 則` : `今日第 ${sentToday}/${dailyLimit} 則`;
          log(`${name} 已送出：${text.split('\n')[0]}（${quota}）`);
          return { channel: name, ok: true };
        }
        const body = await res.text();
        log(`${name} 退回 ${res.status}：${body.slice(0, 200)}`);
        const hint = explain?.(res.status);
        if (hint) log(hint);
        return { channel: name, ok: false, status: res.status, body };
      } catch (err) {
        log(`${name} 連不上：${err.message}`);
        return { channel: name, ok: false, error: err.message };
      }
    },
  };
}
