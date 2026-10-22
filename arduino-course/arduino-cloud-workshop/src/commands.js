/**
 * 瀏覽器送來的字串會被原封不動寫進序列埠，
 * 所以一定要先過白名單 —— 只放行我們自己定義的四個指令。
 *
 * 對應 Arduino 端 p1-monitor-station.ino 的 handleCommand()。
 */
const COMMAND_PATTERN = /^(MUTE|TEST|FLAME|WATER):(\d{1,4})$/;

const RANGES = {
  MUTE:  [0, 1],
  TEST:  [0, 1],
  FLAME: [0, 1023],
  WATER: [0, 1023],
};

export function isValidCommand(input) {
  const text = String(input ?? '').trim();
  const match = COMMAND_PATTERN.exec(text);
  if (!match) return false;
  const [, name, digits] = match;
  const value = Number(digits);
  const [lo, hi] = RANGES[name];
  return value >= lo && value <= hi;
}

export function normalizeCommand(input) {
  return String(input ?? '').trim();
}
