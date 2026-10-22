/*
 * U11 · 不用 delay 也能同時做兩件事
 *
 * 這是整套課程最重要的一個觀念。
 *
 * delay() 會讓整塊板子停住，什麼事都做不了。
 * 只要程式需要「同時」處理兩件節奏不同的事，就不能再用 delay。
 *
 * 接線：
 *   LED_A ── D9  串 220Ω     （每 200 毫秒閃一次）
 *   LED_B ── D10 串 220Ω     （每 1300 毫秒閃一次）
 *   按鈕  ── D2 對 GND        （隨時要能反應）
 */

const int LED_A = 9;
const int LED_B = 10;
const int BUTTON_PIN = 2;

const unsigned long PERIOD_A = 200;
const unsigned long PERIOD_B = 1300;

unsigned long lastA = 0;
unsigned long lastB = 0;
bool stateA = false;
bool stateB = false;

void setup() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  unsigned long now = millis();      // 開機到現在經過幾毫秒

  // 工作一：快閃的燈
  if (now - lastA >= PERIOD_A) {
    lastA = now;
    stateA = !stateA;
    digitalWrite(LED_A, stateA);
  }

  // 工作二：慢閃的燈，和上面互不干擾
  if (now - lastB >= PERIOD_B) {
    lastB = now;
    stateB = !stateB;
    digitalWrite(LED_B, stateB);
  }

  // 工作三：按鈕隨時都能反應，不會被前兩件事卡住
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("按鈕被按下 —— 注意它是立刻回應的");
    delay(200);   // 這裡的 delay 只是簡易去彈跳
  }
}

/*
 * 對照組：用 delay 寫寫看
 *
 *   digitalWrite(LED_A, HIGH); delay(200); digitalWrite(LED_A, LOW); delay(200);
 *   digitalWrite(LED_B, HIGH); delay(1300); ...
 *
 * 你會發現兩顆燈變成輪流閃、不是各自的節奏，
 * 而且按鈕大部分時間根本按不動 —— 因為板子正卡在 delay 裡面。
 *
 * millis() 會在約 49.7 天後歸零，
 * 但因為這裡用的是「相減」而不是「比大小」，溢位時依然算得對。
 */
