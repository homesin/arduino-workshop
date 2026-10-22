import { config } from './config.js';

/**
 * 去處一：存起來。
 * ThingSpeak 免費版最短 15 秒一筆，送太快會被回 0（代表沒收）。
 * 所以這裡自己擋，不是每筆感測值都送。
 */
export function createThingSpeakUploader({ log }) {
  let lastSentAt = 0;
  let okCount = 0;
  let failCount = 0;

  return {
    get stats() {
      return { okCount, failCount };
    },

    async push(reading) {
      if (!config.thingspeakKey) return { skipped: 'no-key' };

      const since = Date.now() - lastSentAt;
      if (since < config.thingspeakIntervalMs) return { skipped: 'too-soon' };
      lastSentAt = Date.now();

      const params = new URLSearchParams({
        api_key: config.thingspeakKey,
        field1: String(reading.t),
        field2: String(reading.h),
        field3: String(reading.flame),
        field4: String(reading.pir),
      });

      try {
        const res = await fetch(config.thingspeakUrl, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: params,
          signal: AbortSignal.timeout(8000),
        });
        const body = (await res.text()).trim();
        // ThingSpeak 成功時回傳這筆是該頻道的第幾筆；回 0 代表被拒絕
        if (res.ok && body !== '0') {
          okCount += 1;
          log(`ThingSpeak 已收第 ${body} 筆（溫度 ${reading.t}°C / 濕度 ${reading.h}%）`);
          return { ok: true, entryId: body };
        }
        failCount += 1;
        log(`ThingSpeak 拒絕了這筆（回傳 "${body}"）。多半是 Write Key 錯，或送太快。`);
        return { ok: false, body };
      } catch (err) {
        failCount += 1;
        log(`ThingSpeak 連不上：${err.message}`);
        return { ok: false, error: err.message };
      }
    },
  };
}
