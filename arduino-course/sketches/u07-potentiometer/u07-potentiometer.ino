/*
 * U07 · 可變電阻：類比輸入與 map()
 *
 * 目標：analogRead 讀到的是 0~1023，怎麼換算成你要的範圍。
 * 接線（10K 可變電阻，三支腳）：
 *   左腳  ── GND
 *   中間腳 ── A0     ← 這支是「刮片」，轉動時電壓會變
 *   右腳  ── 5V
 *   LED 同 U02（D9 串 220Ω）
 */

const int POT_PIN = A0;
const int LED_PIN = 9;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int raw = analogRead(POT_PIN);              // 0 ~ 1023（10 位元）
  int level = map(raw, 0, 1023, 0, 255);      // 換算成 PWM 用的 0 ~ 255
  analogWrite(LED_PIN, level);

  Serial.print("原始值 ");
  Serial.print(raw);
  Serial.print("   亮度 ");
  Serial.println(level);
  delay(100);
}

/*
 * map() 就是等比例換算
 *   map(值, 原本下限, 原本上限, 想要的下限, 想要的上限)
 *   把上下限對調就會變成反向：map(raw, 0, 1023, 255, 0)
 *
 * 為什麼是 1023？
 *   Uno 的類比轉數位是 10 位元，2 的 10 次方 = 1024 階，
 *   所以讀值範圍是 0 到 1023。
 */
