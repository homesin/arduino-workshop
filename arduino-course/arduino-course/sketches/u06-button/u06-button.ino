/*
 * U06 · 按鈕：讀取外界的第一步
 *
 * 目標：INPUT_PULLUP 與「去彈跳」。
 * 接線（不需要外接電阻）：
 *   輕觸開關一腳 ── D2
 *   對角那一腳   ── GND
 *   LED 同 U02（D9 串 220Ω）
 *
 * 為什麼不用接電阻？
 *   INPUT_PULLUP 會啟用晶片內部的上拉電阻，平常把腳位拉到 HIGH，
 *   按下去接到 GND 才變 LOW。所以邏輯是反的：LOW = 有按。
 */

const int BUTTON_PIN = 2;
const int LED_PIN = 9;
const unsigned long DEBOUNCE_MS = 30;

bool ledOn = false;
int lastReading = HIGH;
unsigned long lastChangeAt = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  // 機械接點在接觸瞬間會抖動好幾次，30 毫秒內的變化一律不算數
  if (reading != lastReading) {
    lastChangeAt = millis();
    lastReading = reading;
  }

  static int stableState = HIGH;
  if (millis() - lastChangeAt > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {          // LOW 代表按下去了
      ledOn = !ledOn;                  // 每按一次就切換一次
      digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
      Serial.println(ledOn ? "燈亮" : "燈滅");
    }
  }
}

/*
 * 拿掉去彈跳試試看
 *   把 DEBOUNCE_MS 改成 0，再按幾次。
 *   會發現有時候按一下燈卻切換了兩三次 —— 那就是接點彈跳。
 */
