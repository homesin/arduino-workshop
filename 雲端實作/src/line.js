import { config } from './config.js';
import { createChannelNotifier } from './notifier-core.js';

/**
 * 去處三之一：LINE。
 *
 * 用 broadcast（群發給所有加好友的人），所以不需要知道每個人的 userId，
 * 也不需要對外開 webhook、不需要 ngrok 或 Cloudflare Tunnel。
 *
 * 計費要記住：群發給 N 個好友 = 扣 N 則。
 * 免費方案一個月 200 則，所以這裡強制加了「冷卻時間」和「每日上限」。
 *
 * 這是唯一一家沒設定憑證時仍會「預演」的通道 —— 它是課程主線，
 * 學員還沒申請好帳號時，也要能在終端機看到原本會送出什麼。
 */
export function lineChannel() {
  return {
    name: 'LINE',
    configured: Boolean(config.lineToken),
    dailyLimit: config.lineDailyLimit,
    dryRun: true,

    send(text) {
      return fetch(config.lineUrl, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${config.lineToken}`,
        },
        body: JSON.stringify({ messages: [{ type: 'text', text }] }),
        signal: AbortSignal.timeout(8000),
      });
    },

    explain: (status) => {
      if (status === 401) return '401 = token 錯或過期，回 LINE Developers 重新產生。';
      if (status === 429) return '429 = 這個月的免費額度用完了。';
      return '';
    },
  };
}

/** 單獨使用 LINE（教材前半段、以及自我測試都走這條）。 */
export function createLineNotifier({ log }) {
  return createChannelNotifier({ ...lineChannel(), log });
}
