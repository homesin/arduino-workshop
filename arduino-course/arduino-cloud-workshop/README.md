# 校園環境監測站 — Arduino × Node.js × 雲端

一份感測資料，三個去處：

```
DHT11 / PIR / 火焰感測
        │
   Arduino Uno              每秒送一行 JSON
        │ USB
   Node.js（這個專案）
        │
        ├─ 存起來 ──→ ThingSpeak         長期紀錄、公開圖表
        ├─ 看現在 ──→ 本機即時儀表板      投影用，零額度限制
        └─ 出事叫人 ─→ LINE 官方帳號      異常才推播
```

---

## 五分鐘先跑起來（不需要 Arduino）

```bash
npm install
npm run mock
```

打開 http://localhost:3000 就會看到資料在跳。`--mock` 會產生模擬訊號，
**沒有板子、板子壞了、或是還沒接線，整條流程照樣練得完。**

跑 `npm run selftest` 可以驗證雲端部分的程式是對的 —— 它在本機開一個假的
ThingSpeak 與 LINE 伺服器，檢查我們送出去的網址、標頭、欄位與節流邏輯。

---

## 接上真的 Arduino

### 1. 燒錄 Arduino 程式

用 Arduino IDE 開啟 `arduino/sensor_node/sensor_node.ino`。

先安裝函式庫：**工具 → 管理程式庫 → 搜尋 `SimpleDHT` → 安裝**（作者 Winlin，零相依）。

接線（全部用母/母杜邦線直接插 Uno，**這一段不需要麵包板**）：

| 模組 | VCC | GND | 訊號 |
|---|---|---|---|
| DHT11 溫溼度 | 5V | GND | **D2** |
| HC-SR501 人體紅外線 | 5V | GND | **D3** |
| 火焰感測 | 5V | GND | **A0**（類比） |

上傳後開「序列埠監控視窗」，應該看到每秒一行：

```
{"t":26,"h":58,"pir":0,"flame":812}
```

### 2. 設定 Node.js

```bash
copy .env.example .env      # macOS / Linux 用 cp
npm run ports               # 看看 Arduino 在哪個 COM 埠
```

編輯 `.env`，至少填 `SERIAL_PORT`。另外兩個（ThingSpeak、LINE）**留空也能跑**，
只是那一段會自動略過。

```bash
npm start
```

> **最常見的錯誤**：`Access denied` 或 `Resource temporarily unavailable`
> → Arduino IDE 的序列埠監控視窗還開著，或這支程式已經在跑。
> **一個 COM 埠同時只能有一個程式使用。** 上傳前先 `Ctrl+C` 停掉 Node.js。

---

## 三個去處分別怎麼設定

### 存起來：ThingSpeak

1. 到 <https://thingspeak.com> 註冊（免費，非商用）
2. **Channels → New Channel**，勾選 Field 1~4，分別命名為 溫度／濕度／火焰／人體
3. **API Keys** 分頁，複製 **Write API Key** 填進 `.env` 的 `THINGSPEAK_WRITE_KEY`
4. **Sharing** 分頁選 *Share channel view with everyone* → 學員免登入就看得到

免費方案限制：**4 個頻道、每年 300 萬則、最短 15 秒一筆**。
程式已經內建 15 秒節流，不用擔心送太快被拒絕。

> 一個頻道有 8 個欄位。全班共用一個頻道、每組分到一個欄位，就不會撞到 4 個頻道的上限。

### 看現在：本機儀表板

不用設定，`npm start` 之後開 http://localhost:3000。

資料完全不出這台電腦，**沒有任何額度限制**，投影最順。
同一個區網的其他人也可以用 `http://<你的IP>:3000` 連進來看。

### 出事叫人：LINE

1. <https://developers.line.biz> 建立 **Messaging API** channel（會一併建立 LINE 官方帳號）
2. **Messaging API** 分頁 → 最下面 **Channel access token (long-lived)** → Issue
3. 貼進 `.env` 的 `LINE_CHANNEL_ACCESS_TOKEN`
4. 用手機掃該頁的 QR Code，**把自己的官方帳號加為好友**

程式用的是 **broadcast（群發給所有好友）**，所以：

- 不需要知道任何人的 userId
- **不需要對外開 webhook**，不用 ngrok、不用 Cloudflare Tunnel
- 電腦教室的防火牆不會擋（只是一個往外的 HTTPS 請求）

#### 額度一定要看懂

**群發給 N 個好友 = 扣 N 則。** 免費的「輕用量」方案每月 200 則。

| 情況 | 一次廣播扣 | 200 則夠幾次 |
|---|---|---|
| 一個帳號、10 人加好友 | 10 則 | 20 次 |
| 一個帳號、20 人加好友 | 20 則 | 10 次 |
| **每人自己申請帳號** | 1 則 | **200 次／人** |

課堂上除錯很容易一人試十次，**強烈建議每位學員申請自己的官方帳號**。

程式內建兩道保險：`ALERT_COOLDOWN_SEC`（同類警報的冷卻秒數）
與 `LINE_DAILY_LIMIT`（每天上限，預設 20 則）。兩者在**還沒填 token 的預演模式下
也照樣生效**，所以現場看到的節奏跟正式上線一模一樣。

---

## 火焰感測門檻要現場校正

`.env` 的 `FLAME_THRESHOLD` 預設 400，但**每一顆感測器、每個場地的環境光都不一樣**。

正確做法：先跑起來，看終端機或儀表板上的「A0 原始值」。

- 平常（沒有火）大概是 700~1000
- 拿打火機靠近時會掉到 100~300

取中間值填進 `.env`。**不要照抄預設值就上場。**

---

### 不只 LINE：六家通知服務任選

`.env` 裡**填了憑證的就會啟用**，六家可以同時開，一則警報同時送到每一家。
只想測其中一家時，用 `NOTIFY_CHANNELS=ntfy` 過濾，不必把其他家的憑證刪掉。

| 通道 | 前置作業 | 免費額度 | 要填什麼 |
|---|---|---|---|
| **ntfy** | **完全不用註冊** | 250 則/天 | `NTFY_TOPIC` |
| **Discord** | 建 webhook，3 分鐘 | 30 則/分鐘/webhook | `DISCORD_WEBHOOK_URL` |
| **Telegram** | 跟 @BotFather 對話 | 幾乎無限 | `TELEGRAM_BOT_TOKEN` + `TELEGRAM_CHAT_ID` |
| **Slack** | Incoming webhook | 免費 | `SLACK_WEBHOOK_URL` |
| **LINE** | 建 provider + 官方帳號 | **200 則/月** | `LINE_CHANNEL_ACCESS_TOKEN` |
| **Email** | Gmail 應用程式密碼 | 免費 | `SMTP_*` + `EMAIL_TO` |

想確認某一家有沒有接通，用這支（會真的送出訊息、真的消耗額度）：

```bash
npm run test-alert            # 送給所有已設定的通道
npm run test-alert ntfy       # 只送 ntfy
```

#### ntfy 是最適合學員的一家

手機裝 ntfy App、訂閱一個自己取的主題名稱，往那個網址 POST 就會跳通知，
**連帳號都不用申請**。學員卡在 LINE 官方帳號申請流程時，先用它把整條鏈路跑通。

> **主題名稱等同密碼。** 公開主題任何人知道名稱就能訂閱、也能發訊息進來，
> 所以要加一段隨機字串（例如 `ncyu-arduino-322bbc4a`），不要取 `test` 或 `arduino`。

#### Email 一定要用「應用程式密碼」

Gmail **不接受你的登入密碼**。要到「Google 帳戶 → 安全性 → 兩步驟驗證 →
應用程式密碼」產生一組 16 碼，填進 `SMTP_PASS`。沒開兩步驟驗證的話，
那個選項根本不會出現 —— 這是最多人卡住的地方。

常見錯誤：`EAUTH` 是密碼錯（多半就是用了登入密碼），
`ETIMEDOUT` 是防火牆擋掉 587 埠（部分校園網路會擋外送郵件）。

> Email 是六家裡唯一需要額外套件（`nodemailer`）的一家，也是唯一
> **推播不即時**的一家 —— 要看收件端 App 的收信頻率，而且自動發出的信
> 很容易被丟進垃圾郵件。當備援可以，當主要警報管道會漏掉。

#### Discord 開課前一定要先在教室網路實測

校園網路有可能把 Discord 列在封鎖清單裡。Telegram 同理。

#### LINE Notify 已經不能用了

**2025-03-31 停止服務**，`notify-api.line.me` 已經下線。
網路上多數中文 Arduino + LINE 教學停在 2023、2024 年，寫的都是 LINE Notify 的做法，
照著做只會拿到連線錯誤。現在只剩 Messaging API 這條路。

---
## 檔案結構

```
arduino/sensor_node/sensor_node.ino   Arduino 端：讀感測器、送 JSON
src/config.js                          讀 .env、開場的狀態列
src/sources.js                         序列埠讀取 + 解析；--mock 的模擬訊號
src/thingspeak.js                      去處一：存起來（含 15 秒節流）
src/dashboard.js                       去處二：看現在（Express + Socket.IO）
src/notify.js                          去處三：出事叫人，把下面幾家組起來
src/notifier-core.js                   冷卻、每日上限、預演 —— 每一家共用的規則
src/line.js                            LINE（唯一沒設定也會預演的一家）
src/channels/ntfy.js                   ntfy.sh，不用註冊
src/channels/discord.js                Discord webhook
src/channels/telegram.js               Telegram Bot
src/channels/slack.js                  Slack Incoming Webhook
src/channels/email.js                  Email（SMTP，唯一需要額外套件的一家）
src/index.js                           把上面三個接起來
src/list-ports.js                      npm run ports：列出所有 COM 埠
public/index.html                      儀表板畫面
test/selftest.js                       npm run selftest：40 項自動檢查
```

## 指令

| 指令 | 用途 |
|---|---|
| `npm start` | 正常執行（要接 Arduino） |
| `npm run mock` | 用模擬訊號跑，不需要板子 |
| `npm run ports` | 列出電腦上所有序列埠 |
| `npm run selftest` | 驗證雲端串接邏輯（不需要板子與帳號） |
| `npm run test-alert [通道]` | 真的發一則測試警報，確認通知有沒有接通 |
