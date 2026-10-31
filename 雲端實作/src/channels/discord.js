import { config } from '../config.js';

/**
 * Discord —— 頻道設定裡按「建立 Webhook」複製網址就好，不用開發者帳號。
 *
 * 限制：同一個 webhook 30 則/分鐘、同一個頻道 5 次/5 秒。
 * 我們的警報有冷卻時間，碰不到這個天花板。
 *
 * 開課前要先在教室網路實測 —— 校園網路有可能把 Discord 列在封鎖清單裡。
 */
export function discordChannel() {
  return {
    name: 'Discord',
    configured: Boolean(config.discordWebhookUrl),
    dailyLimit: Infinity,

    send(text) {
      return fetch(config.discordWebhookUrl, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        // Discord 單則上限 2000 字，警報不可能這麼長，保險起見還是切一下
        body: JSON.stringify({ content: text.slice(0, 1900) }),
        signal: AbortSignal.timeout(8000),
      });
    },

    explain: (status) => {
      if (status === 401 || status === 404) return 'webhook 網址錯了，或那個 webhook 已經被刪掉。';
      if (status === 429) return '429 = 送太快（同一個 webhook 限 30 則/分鐘）。';
      return '';
    },
  };
}
