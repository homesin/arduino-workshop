import { config } from './config.js';
import { createChannelNotifier } from './notifier-core.js';
import { lineChannel } from './line.js';
import { ntfyChannel } from './channels/ntfy.js';
import { discordChannel } from './channels/discord.js';
import { slackChannel } from './channels/slack.js';
import { telegramChannel } from './channels/telegram.js';
import { emailChannel } from './channels/email.js';

/**
 * 去處三：出事叫人 —— 可以同時送到好幾家。
 *
 * 規則很簡單：**憑證填了就啟用**。`.env` 裡填了 NTFY_TOPIC 就會送 ntfy，
 * 填了 DISCORD_WEBHOOK_URL 就會送 Discord，六家可以同時開。
 *
 * NOTIFY_CHANNELS 是額外的過濾器，用來「只測其中一家」而不必把其他家的
 * 憑證從 .env 刪掉。例如 NOTIFY_CHANNELS=ntfy 就只會送 ntfy。
 *
 * LINE 是唯一沒設定也會留著的一家（它會在終端機預演），因為那是課程主線。
 */
export function createNotifier({ log }) {
  const everything = [
    lineChannel(),
    ntfyChannel(),
    discordChannel(),
    slackChannel(),
    telegramChannel(),
    emailChannel(),
  ];

  const wanted = config.notifyChannels.length
    ? everything.filter((c) => config.notifyChannels.some((n) => n.toLowerCase() === c.name.toLowerCase()))
    : everything;

  // 沒填憑證的就不列入，免得畫面被四家的預演訊息洗版。LINE 例外。
  const active = wanted.filter((c) => c.configured || c.dryRun);
  const notifiers = active.map((c) => createChannelNotifier({ ...c, log }));

  return {
    /** 給開場橫幅用 */
    channels: active.map((c) => ({ name: c.name, configured: c.configured, dryRun: Boolean(c.dryRun) })),

    get stats() {
      return notifiers.map((n) => n.stats);
    },

    /** 一次送給所有啟用的通道，彼此不互相影響。 */
    async alert(kind, text) {
      const results = await Promise.all(notifiers.map((n) => n.alert(kind, text)));
      return {
        results,
        anyOk: results.some((r) => r.ok),
        allCooldown: results.length > 0 && results.every((r) => r.skipped === 'cooldown'),
      };
    },
  };
}
