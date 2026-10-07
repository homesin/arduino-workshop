/*
 * U01 · 讓板子上的燈閃起來
 *
 * 目標：認識一支 Arduino 程式的三個區塊。
 * 接線：不用接任何東西，用板子內建的 L 燈（接在 D13）。
 */

const int LED_PIN = 13;          // 宣告區：給腳位取名字，之後要改只改這裡

void setup() {
  // setup() 開機後只執行一次，用來做「設定」
  pinMode(LED_PIN, OUTPUT);      // 告訴板子：這支腳要當輸出（送電出去）
}

void loop() {
  // loop() 會一直重複，直到斷電為止
  digitalWrite(LED_PIN, HIGH);   // 送電 → 燈亮
  delay(1000);                   // 停留 1000 毫秒 = 1 秒
  digitalWrite(LED_PIN, LOW);    // 斷電 → 燈滅
  delay(1000);
}

/*
 * 動手改改看
 *   1. 把 delay 改成 200，燈會閃多快？
 *   2. 把兩個 delay 改成不一樣的值，亮的時間比暗的長。
 *   3. 程式沒有「結束」這回事：loop() 跑完會自動再跑一次。
 */
