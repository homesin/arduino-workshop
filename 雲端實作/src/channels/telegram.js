import { config } from '../config.js';

/**
 * Telegram —— 跟 @BotFather 對話就能拿到 bot token，免費且幾乎沒有額度限制。
 *
 * 兩個值都要填：
 *   TELEGRAM_BOT_TOKEN   BotFather 給的
 *   TELEGRAM_CHAT_ID     你自己的 chat id
 *
 * chat id 怎麼拿：先用手機對你的 bot 說一句話，然後開
 *   https://api.telegram.org/bot<TOKEN>/getUpdates
 * 回傳的 JSON 裡 message.chat.id 就是。
 *
 * 沒先跟 bot 說過話的話，bot 是不能主動傳訊息給你的（Telegram 的防騷擾設計）。
 */
export function telegramChannel() {
  return {
    name: 'Telegram',
    configured: Boolean(config.telegramBotToken && config.telegramChatId),
    dailyLimit: Infinity,

    send(text) {
      return fetch(`${config.telegramApiBase}/bot${config.telegramBotToken}/sendMessage`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ chat_id: config.telegramChatId, text }),
        signal: AbortSignal.timeout(8000),
      });
    },

    explain: (status) => {
      if (status === 401) return '401 = bot token 錯了。';
      if (status === 400) return '400 = chat id 錯了，或你還沒先對這個 bot 說過話。';
      return '';
    },
  };
}
