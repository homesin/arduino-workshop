import { config } from '../config.js';

/**
 * ntfy.sh —— 唯一連帳號都不用申請的一家。
 *
 * 手機裝 ntfy App、訂閱一個自己取的主題名稱，然後往那個網址 POST 就會跳通知。
 *
 * 注意：公開主題「任何人知道名稱就能訂閱，也能發訊息進去」。
 * 主題名稱要當密碼用，加一段隨機字串，不要取 test 或 arduino。
 *
 * 中文標題必須走 JSON 發布（POST 到伺服器根路徑），
 * 因為 ntfy 的 X-Title 標頭只吃 ASCII，中文塞進去會變亂碼。
 */
export function ntfyChannel() {
  const [server, topic] = [config.ntfyServer, config.ntfyTopic];

  return {
    name: 'ntfy',
    configured: Boolean(topic),
    dailyLimit: config.ntfyDailyLimit,

    send(text) {
      const [title, ...rest] = text.split('\n');
      const body = rest.join('\n').trim();
      return fetch(server, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          topic,
          title,
          message: body || title,
          tags: ['warning'],
        }),
        signal: AbortSignal.timeout(8000),
      });
    },

    explain: (status) => {
      if (status === 429) return '429 = 超過頻率限制。免費版每日 250 則，且每 5 秒才補一則額度。';
      if (status === 403) return '403 = 這個主題被保護起來了，需要帶存取權杖。';
      return '';
    },
  };
}
