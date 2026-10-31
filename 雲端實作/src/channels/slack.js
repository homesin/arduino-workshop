import { config } from '../config.js';

/**
 * Slack —— Incoming Webhook，跟 Discord 幾乎一模一樣，只差欄位名稱叫 text。
 *
 * 學校如果本來就在用 Slack 或 Teams，這條線最省事：
 * 警報直接進工作用的頻道，不必另外裝 App。
 */
export function slackChannel() {
  return {
    name: 'Slack',
    configured: Boolean(config.slackWebhookUrl),
    dailyLimit: Infinity,

    send(text) {
      return fetch(config.slackWebhookUrl, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text }),
        signal: AbortSignal.timeout(8000),
      });
    },

    explain: (status) => (status === 404 ? 'webhook 網址錯了，或已經被停用。' : ''),
  };
}
