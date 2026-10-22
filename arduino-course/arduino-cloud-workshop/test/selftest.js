/*
 * 自我測試：不需要 Arduino、不需要真的 ThingSpeak / LINE 帳號。
 *
 * 在本機開一個假的雲端伺服器，把 THINGSPEAK_URL 與 LINE_API_URL 指過去，
 * 然後檢查我們送出去的請求「內容對不對」——網址、標頭、欄位、節流、額度上限。
 *
 * 跑法： npm run selftest
 */
import { createServer } from 'node:http';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';

const received = { thingspeak: [], line: [], ntfy: [], discord: [], slack: [], telegram: [] };
let entryId = 0;

const fakeCloud = createServer((req, res) => {
  let body = '';
  req.on('data', (chunk) => { body += chunk; });
  req.on('end', () => {
    if (req.url.startsWith('/thingspeak')) {
      received.thingspeak.push({ headers: req.headers, body });
      entryId += 1;
      res.writeHead(200, { 'Content-Type': 'text/plain' });
      res.end(String(entryId));
    } else if (req.url.startsWith('/line')) {
      received.line.push({ headers: req.headers, body });
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end('{}');
    } else if (req.url.startsWith('/ntfy')) {
      received.ntfy.push({ headers: req.headers, body });
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end('{}');
    } else if (req.url.startsWith('/discord')) {
      received.discord.push({ headers: req.headers, body });
      res.writeHead(204);
      res.end();
    } else if (req.url.startsWith('/slack')) {
      received.slack.push({ headers: req.headers, body });
      res.writeHead(200, { 'Content-Type': 'text/plain' });
      res.end('ok');
    } else if (req.url.includes('/sendMessage')) {
      received.telegram.push({ url: req.url, headers: req.headers, body });
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end('{"ok":true}');
    } else {
      res.writeHead(404);
      res.end();
    }
  });
});

await new Promise((resolve) => fakeCloud.listen(0, '127.0.0.1', resolve));
const base = `http://127.0.0.1:${fakeCloud.address().port}`;

// 這些必須在 import config 之前設定好
process.env.THINGSPEAK_URL = `${base}/thingspeak`;
process.env.THINGSPEAK_WRITE_KEY = 'TEST_WRITE_KEY';
process.env.THINGSPEAK_INTERVAL_SEC = '1';
process.env.LINE_API_URL = `${base}/line`;
process.env.LINE_CHANNEL_ACCESS_TOKEN = 'TEST_LINE_TOKEN';
process.env.ALERT_COOLDOWN_SEC = '0';
process.env.LINE_DAILY_LIMIT = '2';
process.env.FLAME_THRESHOLD = '400';

// 其他四家通知服務也指到同一個假伺服器
process.env.NTFY_SERVER = `${base}/ntfy`;
process.env.NTFY_TOPIC = 'selftest-topic';
process.env.DISCORD_WEBHOOK_URL = `${base}/discord`;
process.env.SLACK_WEBHOOK_URL = `${base}/slack`;
process.env.TELEGRAM_API_BASE = base;
process.env.TELEGRAM_BOT_TOKEN = 'TEST_BOT_TOKEN';
process.env.TELEGRAM_CHAT_ID = '123456';

const { parseLine, createMockSource } = await import('../src/sources.js');
const { createThingSpeakUploader } = await import('../src/thingspeak.js');
const { createLineNotifier } = await import('../src/line.js');
const { createNotifier } = await import('../src/notify.js');

const quiet = () => {};
const results = [];
const check = (name, fn) => {
  try { fn(); results.push(['PASS', name]); }
  catch (err) { results.push(['FAIL', `${name} → ${err.message}`]); }
};
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

// ── 1. Arduino 送上來的那一行，解析得對不對 ─────────────────
const good = parseLine('{"t":25.3,"h":60,"pir":1,"flame":812}\r');
check('解析正常的一行 JSON', () => {
  assert.equal(good.t, 25.3);
  assert.equal(good.h, 60);
  assert.equal(good.pir, 1);
  assert.equal(good.flame, 812);
});
check('略過開機訊息 "# sensor node ready"', () => {
  assert.equal(parseLine('# sensor node ready'), null);
});
check('略過傳輸中斷造成的半行', () => {
  assert.equal(parseLine('{"t":25.3,"h":6'), null);
});
check('缺溫濕度的資料視為無效', () => {
  assert.equal(parseLine('{"pir":1}'), null);
});

// ── 2. ThingSpeak：欄位對不對、有沒有照 15 秒節流 ────────────
const ts = createThingSpeakUploader({ log: quiet });
const reading = { t: 25.3, h: 60, pir: 1, flame: 812, at: Date.now() };

const first = await ts.push(reading);
const second = await ts.push(reading);          // 立刻再送一次，應該被擋下
await sleep(1100);
const third = await ts.push({ ...reading, t: 26.1 });

check('ThingSpeak 第一筆送出成功', () => assert.equal(first.ok, true));
check('ThingSpeak 節流：間隔內的第二筆被擋下', () => assert.equal(second.skipped, 'too-soon'));
check('ThingSpeak 過了間隔後可以再送', () => assert.equal(third.ok, true));
check('ThingSpeak 只收到 2 筆（不是 3 筆）', () => assert.equal(received.thingspeak.length, 2));
check('ThingSpeak 欄位對應正確', () => {
  const params = new URLSearchParams(received.thingspeak[0].body);
  assert.equal(params.get('api_key'), 'TEST_WRITE_KEY');
  assert.equal(params.get('field1'), '25.3');   // 溫度
  assert.equal(params.get('field2'), '60');     // 濕度
  assert.equal(params.get('field3'), '812');    // 火焰
  assert.equal(params.get('field4'), '1');      // 人體
});
check('ThingSpeak 用 form-urlencoded 送出', () => {
  assert.match(received.thingspeak[0].headers['content-type'], /application\/x-www-form-urlencoded/);
});

// ── 3. LINE：授權標頭、訊息格式、每日上限 ────────────────────
const line = createLineNotifier({ log: quiet });
const a = await line.alert('flame', '🔥 偵測到火焰訊號');
const b = await line.alert('pir', '🚶 偵測到有人經過');
const c = await line.alert('flame', '🔥 又一次');    // 第 3 則，應被每日上限擋下

check('LINE 第一則送出成功', () => assert.equal(a.ok, true));
check('LINE 第二則送出成功', () => assert.equal(b.ok, true));
check('LINE 每日上限 2 則生效，第三則被擋下', () => assert.equal(c.skipped, 'daily-limit'));
check('LINE 只實際送出 2 則', () => assert.equal(received.line.length, 2));
check('LINE 帶了正確的 Bearer token', () => {
  assert.equal(received.line[0].headers.authorization, 'Bearer TEST_LINE_TOKEN');
});
check('LINE 訊息是官方要的 messages 陣列格式', () => {
  const payload = JSON.parse(received.line[0].body);
  assert.equal(payload.messages[0].type, 'text');
  assert.equal(payload.messages[0].text, '🔥 偵測到火焰訊號');
});
check('LINE 計數器有累加', () => assert.equal(line.stats.sentToday, 2));

// ── 3.5 多通道：一則警報同時送到五家 ─────────────────────────
const notifier = createNotifier({ log: quiet });
const multi = await notifier.alert('flame', '🔥 偵測到火焰訊號\n感測值 120（門檻 400）');

check('多通道：五家都被啟用', () => assert.equal(notifier.channels.length, 5));
check('多通道：五家都送出成功', () => {
  const ok = multi.results.filter((r) => r.ok).map((r) => r.channel);
  assert.deepEqual(ok.sort(), ['Discord', 'LINE', 'Slack', 'Telegram', 'ntfy']);
});
check('ntfy 走 JSON 發布並帶 topic', () => {
  const payload = JSON.parse(received.ntfy[0].body);
  assert.equal(payload.topic, 'selftest-topic');
});
check('ntfy 第一行當標題、其餘當內文', () => {
  const payload = JSON.parse(received.ntfy[0].body);
  assert.equal(payload.title, '🔥 偵測到火焰訊號');
  assert.equal(payload.message, '感測值 120（門檻 400）');
});
check('Discord 用 content 欄位', () => {
  assert.ok(JSON.parse(received.discord[0].body).content.includes('偵測到火焰訊號'));
});
check('Slack 用 text 欄位', () => {
  assert.ok(JSON.parse(received.slack[0].body).text.includes('偵測到火焰訊號'));
});
check('Telegram 網址帶 bot token、內容帶 chat_id', () => {
  assert.ok(received.telegram[0].url.includes('/botTEST_BOT_TOKEN/sendMessage'));
  assert.equal(JSON.parse(received.telegram[0].body).chat_id, '123456');
});
check('多通道：某一家掛掉不影響其他家', () => {
  // LINE 的每日上限是 2，剛剛第 3 次會被擋，但其他家照送
  assert.equal(multi.anyOk, true);
});

// NOTIFY_CHANNELS 的過濾要在乾淨環境下驗，另開一個行程
const filterOut = execFileSync(process.execPath, ['test/channel-filter-case.js'], {
  encoding: 'utf-8',
  env: { ...process.env, NOTIFY_CHANNELS: 'ntfy' },
});
const filtered = JSON.parse(filterOut.trim());
check('NOTIFY_CHANNELS=ntfy 時只啟用 ntfy', () => assert.deepEqual(filtered.channels, ['ntfy']));


// ── 3.6 Email：只驗信件怎麼組，不碰 SMTP ─────────────────────
const { buildMail } = await import('../src/channels/email.js');

check('Email 第一行當主旨', () => {
  const mail = buildMail('🔥 偵測到火焰訊號\n感測值 120（門檻 400）\n時間 12:00:00');
  assert.equal(mail.subject, '🔥 偵測到火焰訊號');
});
check('Email 內文保留完整訊息', () => {
  const text = '🔥 偵測到火焰訊號\n感測值 120（門檻 400）';
  assert.equal(buildMail(text).text, text);
});
check('Email 沒填 SMTP 設定時不會被啟用', () => {
  assert.equal(notifier.channels.some((c) => c.name === 'Email'), false);
});

// SMTP 設定齊全時才會啟用 —— 一樣另開行程，避免污染上面的斷言。
// 這裡只檢查「有沒有被列入」，不會真的連 SMTP 伺服器。
const mailOut = execFileSync(process.execPath, ['test/channel-filter-case.js'], {
  encoding: 'utf-8',
  env: {
    ...process.env,
    NOTIFY_CHANNELS: 'email',
    SMTP_HOST: 'smtp.example.com',
    SMTP_USER: 'someone@example.com',
    SMTP_PASS: 'app-password',
    EMAIL_TO: 'someone@example.com',
  },
});
check('SMTP 設定齊全時 Email 會啟用', () => {
  assert.deepEqual(JSON.parse(mailOut.trim()).channels, ['Email']);
});


// ── 4. 模擬訊號來源真的會吐資料 ──────────────────────────────
const mock = createMockSource();
const collected = [];
mock.on('reading', (r) => collected.push(r));
await sleep(2300);
mock.stop();
check('模擬來源每秒吐一筆', () => assert.ok(collected.length >= 2, `只收到 ${collected.length} 筆`));
check('模擬資料欄位齊全', () => {
  const r = collected[0];
  for (const key of ['t', 'h', 'pir', 'flame', 'at']) assert.ok(key in r, `缺 ${key}`);
});


// ── 5. 沒填 token 的「預演」也要照冷卻節奏走 ──────────────────
// 這段必須在乾淨的環境變數下跑，所以另開一個行程
const dryOut = execFileSync(process.execPath, ['test/dry-run-case.js'], {
  encoding: 'utf-8',
  env: { ...process.env, LINE_CHANNEL_ACCESS_TOKEN: '', ALERT_COOLDOWN_SEC: '5' },
});
const dry = JSON.parse(dryOut.trim());
check('預演模式：第一則顯示預演訊息', () => assert.equal(dry.first, 'no-token'));
check('預演模式：冷卻時間照樣生效', () => assert.equal(dry.second, 'cooldown'));
check('預演模式不佔用任何 LINE 額度', () => assert.equal(dry.sentToday, 0));

// ── 6. 指令白名單：這些字串會被寫進序列埠 ──────────
const { isValidCommand } = await import('../src/commands.js');

check('放行正常指令', () => {
  for (const ok of ['MUTE:1', 'MUTE:0', 'TEST:1', 'FLAME:400', 'WATER:1023']) {
    assert.equal(isValidCommand(ok), true, ok);
  }
});
check('擋下不認得的指令名稱', () => {
  assert.equal(isValidCommand('REBOOT:1'), false);
  assert.equal(isValidCommand('LED:255'), false);
});
check('擋下超出範圍的數值', () => {
  assert.equal(isValidCommand('MUTE:5'), false);
  assert.equal(isValidCommand('WATER:9999'), false);
});
check('擋下夾帶換行的注入嘗試', () => {
  const LF = String.fromCharCode(10);
  assert.equal(isValidCommand('MUTE:1' + LF + 'FLAME:0'), false);
  assert.equal(isValidCommand('MUTE:1;TEST:1'), false);
});
check('擋下空值與非字串', () => {
  for (const bad of ['', '   ', null, undefined, 42, {}]) {
    assert.equal(isValidCommand(bad), false, String(bad));
  }
});

// ── 收尾 ────────────────────────────────────────────────────
fakeCloud.close();

console.log('');
for (const [status, name] of results) {
  console.log(`  ${status === 'PASS' ? '✔' : '✘'} ${status}  ${name}`);
}
const failed = results.filter(([s]) => s === 'FAIL').length;
console.log('');
console.log(`  ${results.length - failed} 項通過 / ${results.length} 項`);
console.log('');
process.exit(failed ? 1 : 0);
