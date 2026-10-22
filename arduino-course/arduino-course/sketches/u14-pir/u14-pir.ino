/*
 * U14 · 人體紅外線 HC-SR501
 *
 * 目標：認識「有狀態的數位感測器」，以及邊緣觸發。
 * 接線：
 *   VCC ── 5V     GND ── GND     OUT ── D3
 *   蜂鳴器 ── D8（選用）
 *
 * 模組上有兩顆旋鈕：
 *   一顆調靈敏度（偵測距離），一顆調延遲時間（觸發後 HIGH 維持多久）。
 *   實作時把延遲那顆逆時針轉到底，反應最快。
 */

const int PIR_PIN = 3;
const int BUZZER_PIN = 8;

bool lastMotion = false;
unsigned long bootAt = 0;
const unsigned long WARMUP_MS = 60000UL;   // 開機暖機約一分鐘

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  bootAt = millis();
  Serial.println("PIR 暖機中，前一分鐘的讀值不要相信");
}

void loop() {
  bool warmedUp = millis() - bootAt > WARMUP_MS;
  bool motion = digitalRead(PIR_PIN) == HIGH;

  // 只在「狀態改變的那一瞬間」動作，不是每次迴圈都印
  if (motion != lastMotion) {
    lastMotion = motion;
    if (!warmedUp) {
      Serial.println("（暖機中，忽略）");
    } else if (motion) {
      Serial.println("偵測到有人");
      tone(BUZZER_PIN, 880, 150);
    } else {
      Serial.println("恢復無人");
    }
  }

  delay(50);
}

/*
 * 為什麼要判斷「狀態改變」
 *   PIR 觸發後會維持 HIGH 好幾秒。
 *   如果每次迴圈都印一次，序列埠會被洗版；
 *   如果每次都發一則 LINE，額度幾秒就用光。
 *
 * 這個「只在變化時動作」的寫法，在後面的雲端通報會再用一次。
 */
