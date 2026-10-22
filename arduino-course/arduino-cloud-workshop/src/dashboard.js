import express from 'express';
import { createServer } from 'node:http';
import { Server } from 'socket.io';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { config } from './config.js';
import { isValidCommand, normalizeCommand } from './commands.js';

const publicDir = join(dirname(fileURLToPath(import.meta.url)), '..', 'public');

/**
 * 去處二：看現在。
 * 資料完全不出這台電腦，所以沒有任何額度限制，投影用最順。
 */
export function createDashboard({ log, onCommand }) {
  const app = express();
  const http = createServer(app);
  const io = new Server(http);

  const history = []; // 只留最近 120 筆，夠畫兩分鐘的曲線
  const events = [];  // 最近的警報記錄

  app.use(express.static(publicDir));
  app.get('/api/readings', (_req, res) => res.json(history));

  io.on('connection', (socket) => {
    socket.emit('config', { flameThreshold: config.flameThreshold });
    socket.emit('history', history);
    socket.emit('events', events);

    // 瀏覽器 -> Node.js -> Arduino
    socket.on('command', (command) => {
      const text = normalizeCommand(command);
      // 這個字串會被直接寫進序列埠，所以只放行白名單內的指令
      if (!isValidCommand(text)) {
        log(`忽略不合法的指令：${text.slice(0, 40)}`);
        return;
      }
      log(`收到瀏覽器指令 ${text}`);
      const sent = onCommand ? onCommand(text) : false;
      socket.emit('commandResult', { command: text, sent });
    });
  });

  return {
    start() {
      return new Promise((resolve, reject) => {
        http.once('error', (err) => {
          if (err.code === 'EADDRINUSE') {
            log(`連接埠 ${config.port} 已經被占用了。`);
            log('多半是上一次的程式還在跑（另一個終端機視窗？），先把它關掉；');
            log(`或是在 .env 把 PORT 改成別的數字，例如 PORT=3001。`);
            process.exit(1);
          }
          reject(err);
        });
        http.listen(config.port, () => {
          log(`儀表板已開在 http://localhost:${config.port}`);
          resolve();
        });
      });
    },

    push(reading) {
      history.push(reading);
      if (history.length > 120) history.shift();
      io.emit('reading', reading);
    },

    pushEvent(event) {
      events.unshift(event);
      if (events.length > 20) events.pop();
      io.emit('event', event);
    },

    stop() {
      io.close();
      http.close();
    },
  };
}
