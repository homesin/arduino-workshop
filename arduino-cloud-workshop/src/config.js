import 'dotenv/config';

const num = (value, fallback) => {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
};

const list = (value) => (value || '').split(',').map((s) => s.trim()).filter(Boolean);

export const config = {
  serialPort: process.env.SERIAL_PORT || 'COM3',
  serialBaud: num(process.env.SERIAL_BAUD, 9600),

  thingspeakKey: (process.env.THINGSPEAK_WRITE_KEY || '').trim(),
  thingspeakUrl: process.env.THINGSPEAK_URL || 'https://api.thingspeak.com/update',
  thingspeakIntervalMs: num(process.env.THINGSPEAK_INTERVAL_SEC, 15) * 1000,

  // ── 通知去處 ────────────────────────────────────────────────
  // 憑證填了就啟用，留空就略過。NOTIFY_CHANNELS 有填的話再多一層過濾，
  // 用來「只測其中一家」而不必把其他家的憑證刪掉。
  notifyChannels: list(process.env.NOTIFY_CHANNELS),

  lineToken: (process.env.LINE_CHANNEL_ACCESS_TOKEN || '').trim(),
  lineUrl: process.env.LINE_API_URL || 'https://api.line.me/v2/bot/message/broadcast',
  lineDailyLimit: num(process.env.LINE_DAILY_LIMIT, 20),

  ntfyServer: (process.env.NTFY_SERVER || 'https://ntfy.sh').replace(/\/+$/, ''),
  ntfyTopic: (process.env.NTFY_TOPIC || '').trim(),
  ntfyDailyLimit: num(process.env.NTFY_DAILY_LIMIT, 200),

  discordWebhookUrl: (process.env.DISCORD_WEBHOOK_URL || '').trim(),
  slackWebhookUrl: (process.env.SLACK_WEBHOOK_URL || '').trim(),

  telegramApiBase: process.env.TELEGRAM_API_BASE || 'https://api.telegram.org',
  telegramBotToken: (process.env.TELEGRAM_BOT_TOKEN || '').trim(),
  telegramChatId: (process.env.TELEGRAM_CHAT_ID || '').trim(),

  // Gmail 要用「應用程式密碼」，不是登入密碼
  smtpHost: (process.env.SMTP_HOST || '').trim(),
  smtpPort: num(process.env.SMTP_PORT, 587),
  smtpUser: (process.env.SMTP_USER || '').trim(),
  smtpPass: process.env.SMTP_PASS || '',
  emailFrom: (process.env.EMAIL_FROM || '').trim(),
  emailTo: (process.env.EMAIL_TO || '').trim(),
  emailDailyLimit: num(process.env.EMAIL_DAILY_LIMIT, 50),

  port: num(process.env.PORT, 3000),
  flameThreshold: num(process.env.FLAME_THRESHOLD, 400),
  alertCooldownMs: num(process.env.ALERT_COOLDOWN_SEC, 60) * 1000,
};

/** 開場先把設定印出來，讓學員一眼看出哪一段還沒接上。 */
export function printStartupBanner({ mock, channels = [] }) {
  const mark = (on) => (on ? '✔ 已啟用' : '— 未設定（略過）');
  console.log('');
  console.log('  Arduino 雲端實作 · Node.js 中樞');
  console.log('  ─────────────────────────────────');
  console.log(`  資料來源      ${mock ? '模擬訊號（--mock，沒有接板子也能跑）' : `序列埠 ${config.serialPort} @ ${config.serialBaud}`}`);
  console.log(`  即時儀表板    ✔ http://localhost:${config.port}`);
  console.log(`  ThingSpeak    ${mark(config.thingspeakKey)}`);
  if (channels.length === 0) {
    console.log('  警報通知      — 一家都沒設定（會在終端機預演）');
  } else {
    for (const ch of channels) {
      const state = ch.configured ? '✔ 已啟用' : '— 未設定（終端機預演）';
      console.log(`  警報通知      ${state}  ${ch.name}`);
    }
  }
  console.log('  ─────────────────────────────────');
  console.log('  按 Ctrl+C 結束。要用 Arduino IDE 上傳程式前，一定要先結束這支程式。');
  console.log('');
}
