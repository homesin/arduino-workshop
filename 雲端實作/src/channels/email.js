import nodemailer from 'nodemailer';
import { config } from '../config.js';

/**
 * Email —— 唯一「人人都有」的收訊管道，不必裝任何 App。
 *
 * 代價是：手機推播不一定即時（要看信箱 App 的收信頻率），
 * 而且自動發出的信很容易被丟進垃圾郵件。警報用它當備援可以，
 * 當主要管道會漏掉。
 *
 * Gmail 要注意：**不能用你的登入密碼**，必須另外產生「應用程式密碼」
 * （Google 帳戶 → 安全性 → 兩步驟驗證 → 應用程式密碼，16 碼）。
 * 沒有開兩步驟驗證的話，這個選項不會出現。
 *
 * SMTP_PASS 請自己填進 .env，不要貼給任何人，也不要 commit 進版本庫。
 */

/** 把一段警報文字拆成信件的主旨與內文。獨立出來才好測試。 */
export function buildMail(text) {
  const [subject, ...rest] = text.split('\n');
  return {
    subject: subject.trim(),
    text,                          // 內文保留完整訊息，含第一行
    body: rest.join('\n').trim(),  // 只有第一行以外的部分，測試用
  };
}

export function emailChannel() {
  const ready = Boolean(config.smtpHost && config.smtpUser && config.smtpPass && config.emailTo);

  // 沒設定就不要建 transporter，免得啟動時就去連 SMTP
  const transporter = ready
    ? nodemailer.createTransport({
        host: config.smtpHost,
        port: config.smtpPort,
        secure: config.smtpPort === 465, // 465 走 SSL，587 走 STARTTLS
        auth: { user: config.smtpUser, pass: config.smtpPass },
      })
    : null;

  return {
    name: 'Email',
    configured: ready,
    dailyLimit: config.emailDailyLimit,

    async send(text) {
      const mail = buildMail(text);
      await transporter.sendMail({
        from: config.emailFrom || config.smtpUser,
        to: config.emailTo,
        subject: mail.subject,
        text: mail.text,
      });
      // 其他通道回傳的是 fetch 的 Response，這裡湊出同樣的形狀讓共用核心能處理。
      // sendMail 失敗會直接 throw，由 notifier-core 的 try/catch 接住。
      return { ok: true };
    },

    // SMTP 的錯誤不是 HTTP 狀態碼，會以例外的形式被 notifier-core 接住，
    // 所以這裡不需要 explain()。常見的是 EAUTH（密碼錯，多半是用了登入密碼
    // 而不是應用程式密碼）和 ETIMEDOUT（防火牆擋掉 587 埠）。
  };
}
