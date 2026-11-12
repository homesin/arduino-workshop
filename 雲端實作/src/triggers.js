/**
 * 警報判斷（純函式，不碰網路，selftest 直接測它）。
 *  - 光線被遮住：光敏讀值大於門檻（和 c04d 一樣，數字越大越暗）
 *  - 濕度突然升高：目前濕度比「最近一段時間的平均」高出門檻以上（對 DHT11 呼氣就會這樣）
 */

export function createHumidityWatcher({ windowSize = 30, minSamples = 5, rise = 8 } = {}) {
  const history = [];
  return {
    /** 回傳 { rise, baseline }，濕度突然升高時才有值；否則回傳 null。每一筆讀值都要餵進來。 */
    push(h) {
      let hit = null;
      if (Number.isFinite(h)) {
        if (history.length >= minSamples) {
          const baseline = history.reduce((a, b) => a + b, 0) / history.length;
          if (h - baseline >= rise) hit = { rise: Math.round((h - baseline) * 10) / 10, baseline: Math.round(baseline * 10) / 10 };
        }
        history.push(h);
        if (history.length > windowSize) history.shift();
      }
      return hit;
    },
  };
}

/** 光敏讀值大於門檻（暗）就是被遮住；缺值或非數字不算。 */
export function isCovered(light, darkAbove) {
  return Number.isFinite(light) && light > darkAbove;
}
