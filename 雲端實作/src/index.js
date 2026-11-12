import { config, printStartupBanner } from './config.js';
import { createSerialSource, createMockSource } from './sources.js';
import { createThingSpeakUploader } from './thingspeak.js';
import { createNotifier } from './notify.js';
import { createDashboard } from './dashboard.js';
import { createHumidityWatcher, isCovered } from './triggers.js';

const mock = process.argv.includes('--mock');

const stamp = () => new Date().toLocaleTimeString('zh-TW', { hour12: false });
const log = (msg) => console.log(`[${stamp()}] ${msg}`);

// 先建通知器，開場橫幅要列出有哪幾家通道被啟用
const notifier = createNotifier({ log });

printStartupBanner({ mock, channels: notifier.channels });

// 先宣告，因為儀表板要能把指令轉給它，而它又要在儀表板之後才建立
let source = null;

const dashboard = createDashboard({
  log,
  onCommand: (command) => (source && source.send ? source.send(command) : false),
});
await dashboard.start();

const thingspeak = createThingSpeakUploader({ log });

source = mock ? createMockSource() : createSerialSource();

source.on('status', (msg) => log(msg));
source.on('fatal', (msg) => {
  log(`序列埠錯誤：${msg}`);
  log('提示：先跑 npm run mock，沒有板子也能把後面三段全部練完。');
  process.exit(1);
});

let printedFirst = false;
const humidityWatcher = createHumidityWatcher({ rise: config.humidityRisePct });

source.on('reading', async (reading) => {
  if (!printedFirst) {
    printedFirst = true;
    log(`收到第一筆資料，接下來每秒一筆：溫度 ${reading.t}°C、濕度 ${reading.h}%`);
  }

  // ── 去處二：即時儀表板（每一筆都送，本機不用省）
  dashboard.push(reading);

  // ── 去處一：ThingSpeak（自動節流到 15 秒一筆）
  thingspeak.push(reading).catch((err) => log(`ThingSpeak 例外：${err.message}`));

  // ── 去處三：警報通知（只有異常才送，而且有冷卻與每日上限）
  // dist 是公分；-1 代表沒收到回波，不能當成「很近」
  if (reading.dist >= 0 && reading.dist < config.nearCm) {
    await fireAlert('near', `📏 有東西靠近\n距離 ${reading.dist} 公分（門檻 ${config.nearCm}）\n時間 ${stamp()}`);
  }
  if (isCovered(reading.light, config.lightDarkAbove)) {
    await fireAlert('dark', `🌑 光線被遮住
光敏讀值 ${reading.light}（大於 ${config.lightDarkAbove} 算暗）
時間 ${stamp()}`);
  }
  const wet = humidityWatcher.push(reading.h);
  if (wet) {
    await fireAlert('humid', `💨 濕度突然升高
目前 ${reading.h}%，比最近平均 ${wet.baseline}% 高 ${wet.rise} 個百分點
時間 ${stamp()}`);
  }
  if (reading.pir === 1) {
    await fireAlert('pir', `🚶 偵測到有人經過\n當下溫度 ${reading.t}°C、濕度 ${reading.h}%\n時間 ${stamp()}`);
  }
});

async function fireAlert(kind, text) {
  const result = await notifier.alert(kind, text);
  if (result.allCooldown) return; // 全部都在冷卻中就安靜，不洗版面

  const delivered = result.results.filter((r) => r.ok).map((r) => r.channel);
  const dryRun = result.results.filter((r) => r.skipped === 'no-token').map((r) => r.channel);
  const limited = result.results.filter((r) => r.skipped === 'daily-limit').map((r) => r.channel);

  const notes = [];
  if (delivered.length) notes.push(`已送達 ${delivered.join('、')}`);
  if (dryRun.length) notes.push(`${dryRun.join('、')} 未設定（預演）`);
  if (limited.length) notes.push(`${limited.join('、')} 已達今日上限`);

  dashboard.pushEvent({
    kind,
    text: text.split('\n')[0],
    at: Date.now(),
    delivered: result.anyOk,
    note: notes.join('　'),
  });
}

// 每 30 秒回報一次狀態，讓學員知道程式還活著、額度用了多少
setInterval(() => {
  const ts = thingspeak.stats;
  const quota = notifier.stats
    .map((s) => `${s.name} ${s.sentToday}${s.dailyLimit === Infinity ? '' : `/${s.dailyLimit}`}`)
    .join('、');
  log(`狀態：ThingSpeak 成功 ${ts.okCount} / 失敗 ${ts.failCount}　通知 ${quota || '（未啟用）'}`);
}, 30_000).unref();

const shutdown = () => {
  console.log('');
  log('收工，序列埠已釋放。現在可以用 Arduino IDE 上傳程式了。');
  source.stop?.();
  dashboard.stop();
  process.exit(0);
};
process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
