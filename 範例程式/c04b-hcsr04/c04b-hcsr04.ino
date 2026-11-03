/*
 * 第四章 · 感測器 2：HC-SR04 超音波測距
 *
 * 目標：不用函式庫，自己算時間差換成距離。
 * 接線（四支針腳）：
 *   VCC ── 紅軌（5V）   GND ── 藍軌（GND）
 *   Trig ── Uno D4   Echo ── Uno D5
 *
 * 原理：Trig 送出一個 10 微秒的脈衝 → 模組發出 8 個超音波脈衝
 *       → 聲波撞到物體反彈 → Echo 腳變成 HIGH，持續時間 = 聲音來回飛行的時間
 */

const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
const int NEAR_CM = 30;           // 判斷門檻：小於幾公分算「有東西靠近」（要依現場調整）

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("HC-SR04 開始量測");
}

// 回傳公分；量不到時回傳 -1
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);          // 這 10 微秒就是「發射」指令
  digitalWrite(TRIG_PIN, LOW);

  // 等 Echo 變 HIGH 並量它持續多久，最多等 30 毫秒（約 5 公尺）
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1;   // 逾時：太遠、沒接好，或聲波沒反彈回來

  // 聲速約每秒 343 公尺 = 每微秒 0.0343 公分
  // 除以 2 是因為量到的是「來回」
  return duration * 0.0343f / 2.0f;
}

void loop() {
  float cm = readDistanceCm();

  if (cm < 0) {
    Serial.println("量不到（-1）：超出範圍，或 VCC／Trig／Echo 沒接好");
  } else {
    Serial.print("距離 ");
    Serial.print(cm, 1);
    Serial.print(" 公分");
    if (cm < NEAR_CM) {
      Serial.print("    ← 有東西靠近");
    }
    Serial.println();
  }

  delay(300);                     // 兩次量測之間要留間隔，避免上一次的回音干擾
}

/*
 * 動手試試
 *   手掌放在探頭前 10～30 公分，慢慢移近、移遠，看數字怎麼變。
 *   把手移到 30 公分內，看有沒有出現「有東西靠近」。
 *
 * 一直是 -1（量不到）
 *   先查 VCC 有沒有接到 5V、GND 有沒有接好，再查 Trig 與 Echo 有沒有接成 D4、D5。
 *   前方完全空曠、超過 4 公尺也會是 -1，這是正常的。
 *
 * 量不準的常見原因
 *   物體表面是布、海綿等吸音材質 → 聲波被吃掉
 *   物體是斜面 → 聲波反彈到別的方向
 *   距離小於 2 公分 → 超出模組的下限
 */
