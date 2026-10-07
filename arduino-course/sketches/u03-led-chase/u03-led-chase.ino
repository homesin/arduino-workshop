/*
 * U03 · 五顆燈的跑馬燈
 *
 * 目標：用陣列和 for 迴圈，取代五份複製貼上的程式碼。
 * 接線：五顆 LED 各串一顆 220Ω 電阻，分別接 D4 D5 D6 D7 D8，
 *       負極全部接到麵包板同一條 GND 匯流排。
 */

const int LED_PINS[] = {4, 5, 6, 7, 8};      // 陣列：把五個腳位收成一組
const int LED_COUNT = 5;
const int STEP_MS = 120;                      // 每一格停留多久

void setup() {
  // 用迴圈一次設定五支腳，不必寫五行 pinMode
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
}

void loop() {
  // 從左跑到右
  for (int i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    delay(STEP_MS);
    digitalWrite(LED_PINS[i], LOW);
  }
  // 再從右跑回左（頭尾不重複點亮，看起來比較順）
  for (int i = LED_COUNT - 2; i > 0; i--) {
    digitalWrite(LED_PINS[i], HIGH);
    delay(STEP_MS);
    digitalWrite(LED_PINS[i], LOW);
  }
}

/*
 * 動手改改看
 *   1. 把 STEP_MS 調小，跑馬燈變快
 *   2. 讓兩顆燈同時亮（點亮 i 的時候不要關掉 i-1）
 *   3. 加第六顆燈：只要在陣列補一個腳位、把 LED_COUNT 改成 6
 */
