/*
 * U02 · 接一顆自己的 LED
 *
 * 目標：認識極性與限流電阻。
 * 接線：
 *   D9 ──[220Ω 電阻]── LED 長腳(正極)
 *   LED 短腳(負極) ── GND
 *
 * 電阻接哪一邊都可以，本身不分方向；LED 分。
 * 沒有電阻會讓 LED 在幾秒到幾分鐘內燒掉。
 */

const int LED_PIN = 9;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}

/*
 * 燈不亮的時候，照這個順序查
 *   1. LED 長短腳接反了 —— 九成是這個，轉個方向再試
 *   2. 線接在 D8 而不是 D9
 *   3. 麵包板中間那條溝，兩側是不相通的
 *   4. LED 本身壞了，換一顆
 */
