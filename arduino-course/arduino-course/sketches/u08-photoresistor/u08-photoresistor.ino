/*
 * U08 · 光敏電阻：自動夜燈
 *
 * 目標：分壓電路，以及「門檻要現場量」這件事。
 * 接線（光敏電阻本身不分方向）：
 *   5V ── 光敏電阻 ── A0 ── 10KΩ 電阻 ── GND
 *   LED 同 U02（D9 串 220Ω）
 *
 * 光敏電阻在亮的地方電阻小、暗的地方電阻大，
 * 和 10K 電阻串起來後，A0 量到的就是中間那一點的電壓。
 */

const int LDR_PIN = A0;
const int LED_PIN = 9;

// 這個值一定要現場量過再改，不要照抄
const int DARK_THRESHOLD = 400;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int light = analogRead(LDR_PIN);
  bool isDark = light < DARK_THRESHOLD;
  digitalWrite(LED_PIN, isDark ? HIGH : LOW);

  Serial.print("亮度 ");
  Serial.print(light);
  Serial.println(isDark ? "   → 暗，開燈" : "   → 亮，關燈");
  delay(200);
}

/*
 * 門檻怎麼定
 *   1. 先上傳，開序列埠監控視窗
 *   2. 記下「室內正常照明」的讀值
 *   3. 用手蓋住感測器，記下「遮住」的讀值
 *   4. 取兩者中間值填進 DARK_THRESHOLD
 *
 * 每一顆光敏電阻、每個場地的環境光都不一樣，
 * 這是所有類比感測器共通的規矩。
 */
