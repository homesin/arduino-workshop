import { EventEmitter } from 'node:events';
import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';
import { config } from './config.js';

/**
 * Arduino 每秒送一行 JSON 上來，長這樣：
 *   {"t":25.3,"h":60,"pir":0,"flame":812}
 * 這裡負責把那一行變成物件，並丟出 reading 事件。
 */
function parseLine(line) {
  const text = line.trim();
  if (!text.startsWith('{')) return null; // 開機訊息之類的雜訊直接略過
  let raw;
  try {
    raw = JSON.parse(text);
  } catch {
    return null; // 傳輸中斷造成的半行，丟掉就好
  }
  const t = Number(raw.t);
  const h = Number(raw.h);
  const flame = Number(raw.flame);
  if (!Number.isFinite(t) || !Number.isFinite(h)) return null;
  const reading = {
    t: Math.round(t * 10) / 10,
    h: Math.round(h * 10) / 10,
    pir: raw.pir ? 1 : 0,
    flame: Number.isFinite(flame) ? flame : 1023,
    at: Date.now(),
  };
  // 整合專題（p1-monitor-station）還會多送這幾個欄位，有就一起帶上
  for (const key of ['light', 'water', 'dist', 'status']) {
    if (Number.isFinite(Number(raw[key]))) reading[key] = Number(raw[key]);
  }
  return reading;
}

/** 真的接了板子時用這個。 */
export function createSerialSource() {
  const bus = new EventEmitter();
  const port = new SerialPort({ path: config.serialPort, baudRate: config.serialBaud });
  const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

  port.on('open', () => bus.emit('status', `已連上 ${config.serialPort}`));
  port.on('error', (err) => bus.emit('fatal', explainSerialError(err)));
  parser.on('data', (line) => {
    const reading = parseLine(line);
    if (reading) bus.emit('reading', reading);
    else if (line.trim()) bus.emit('status', `略過非資料行：${line.trim().slice(0, 60)}`);
  });

  // 電腦 -> Arduino：一行一個指令，例如 "MUTE:1"
  bus.send = (command) => {
    if (!port.isOpen) return false;
    port.write(`${command}
`);
    return true;
  };
  bus.stop = () => port.isOpen && port.close(() => {});
  return bus;
}

/** 沒有板子、或板子壞了的時候用 `npm run mock`，整條流程照樣跑得完。 */
export function createMockSource() {
  const bus = new EventEmitter();
  let tick = 0;
  const timer = setInterval(() => {
    tick += 1;
    // 溫度慢慢在 24~30 度之間來回，濕度反向擺動，看起來像真的
    const t = 27 + Math.sin(tick / 20) * 3 + (Math.random() - 0.5) * 0.4;
    const h = 60 - Math.sin(tick / 20) * 8 + (Math.random() - 0.5) * 2;
    // 每 25 秒模擬一次有人經過，每 40 秒模擬一次火焰，方便測警報
    const pir = tick % 25 === 0 ? 1 : 0;
    const flame = tick % 40 === 0 ? 180 : 800 + Math.round(Math.random() * 100);
    bus.emit('reading', {
      t: Math.round(t * 10) / 10,
      h: Math.round(h * 10) / 10,
      pir,
      flame,
      at: Date.now(),
    });
  }, 1000);

  setTimeout(() => bus.emit('status', '模擬訊號啟動（每秒一筆）'), 0);
  bus.send = (command) => {
    bus.emit('status', `[模擬] 收到指令 ${command}（沒有接板子，只印出來）`);
    return true;
  };
  bus.stop = () => clearInterval(timer);
  return bus;
}

function explainSerialError(err) {
  const message = String(err?.message || err);
  if (/Access denied|Resource temporarily unavailable/i.test(message)) {
    return `${config.serialPort} 被占用了。Arduino IDE 的序列埠監控視窗還開著嗎？關掉它再跑一次。`;
  }
  if (/File not found|cannot open|No such file/i.test(message)) {
    return `找不到 ${config.serialPort}。跑 npm run ports 看看實際的埠號，再改 .env。`;
  }
  return message;
}

export { parseLine };
