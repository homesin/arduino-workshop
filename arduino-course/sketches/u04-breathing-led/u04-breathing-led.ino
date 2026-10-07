/*
 * U04 · 呼吸燈
 *
 * 目標：從「開或關」走到「亮到多少」，認識 PWM。
 * 接線：同 U02（D9 串 220Ω 到 LED 長腳）。
 *
 * 重點：只有腳位旁邊印著 ~ 的才支援 analogWrite，
 *       在 Uno 上是 D3 D5 D6 D9 D10 D11。
 */

const int LED_PIN = 9;
const int STEP = 5;              // 每次亮度變化多少（1~255，越小越平滑）
const int HOLD_MS = 10;          // 每一階停留幾毫秒

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // 逐漸變亮：0 → 255
  for (int level = 0; level <= 255; level += STEP) {
    analogWrite(LED_PIN, level);
    delay(HOLD_MS);
  }
  // 逐漸變暗：255 → 0
  for (int level = 255; level >= 0; level -= STEP) {
    analogWrite(LED_PIN, level);
    delay(HOLD_MS);
  }
}

/*
 * PWM 到底在做什麼
 *   它不是把電壓降低，而是快速地開開關關。
 *   analogWrite(9, 128) 表示「一半的時間通電」，
 *   切換速度約每秒 490 次，眼睛看不出閃爍，只覺得比較暗。
 *
 * 動手改改看
 *   1. STEP 改成 25，會看到一階一階的跳動
 *   2. HOLD_MS 改成 30，呼吸變慢
 */
