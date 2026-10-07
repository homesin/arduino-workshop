/*
 * U13 · 超音波測距 HC-SR04
 *
 * 目標：不用函式庫，自己算時間差。
 * 接線：
 *   VCC ── 5V     GND ── GND
 *   Trig ── D4    Echo ── D5
 *
 * 原理：Trig 送出一個 10 微秒的脈衝 → 模組發出 8 個超音波脈衝
 *       → 聲波撞到物體反彈 → Echo 腳變成 HIGH，持續時間 = 來回飛行時間
 */

const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
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
  if (duration == 0) return -1;   // 逾時：太遠、或聲波沒反彈回來

  // 聲速約每秒 343 公尺 = 每微秒 0.0343 公分
  // 除以 2 是因為量到的是「來回」
  return duration * 0.0343f / 2.0f;
}

void loop() {
  float cm = readDistanceCm();

  if (cm < 0) {
    Serial.println("量不到（超出範圍或表面吸音）");
  } else {
    Serial.print("距離 ");
    Serial.print(cm, 1);
    Serial.println(" 公分");
  }

  delay(200);   // 兩次量測之間要留間隔，避免上一次的回音干擾
}

/*
 * 量不準的常見原因
 *   物體表面是布、海綿等吸音材質 → 聲波被吃掉
 *   物體是斜面 → 聲波反彈到別的方向
 *   距離小於 2 公分 → 模組本身的下限
 *   兩次量測間隔太短 → 上一次的回音還在
 */
