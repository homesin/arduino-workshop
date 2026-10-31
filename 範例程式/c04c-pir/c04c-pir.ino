/*
 * 第四章 · 感測器 3：HC-SR501 人體紅外線
 *
 * 目標：認識「有狀態的數位感測器」，以及邊緣觸發（只在狀態改變的瞬間動作）。
 * 接線（三支針腳）：
 *   VCC ── 5V     GND ── GND     OUT ── D3
 *
 * 原理：偵測人體發出的紅外線「變化」。有人移動，OUT 就變 HIGH；一段時間後自己變回 LOW。
 *
 * 模組上有兩顆旋鈕：
 *   一顆調靈敏度（偵測距離），一顆調延遲時間（觸發後 HIGH 維持多久）。
 *   上課時把延遲那顆逆時針轉到底，反應最快。
 */

const int PIR_PIN = 3;

bool lastMotion = false;
unsigned long bootAt = 0;
const unsigned long WARMUP_MS = 60000UL;   // 剛插電要暖機約一分鐘

void setup() {
  pinMode(PIR_PIN, INPUT);
  Serial.begin(9600);
  bootAt = millis();              // millis() 是「開機到現在經過幾毫秒」
  Serial.println("PIR 暖機中，前一分鐘的讀值不要相信");
}

void loop() {
  bool warmedUp = millis() - bootAt > WARMUP_MS;
  bool motion = digitalRead(PIR_PIN) == HIGH;

  // 只在「狀態改變的那一瞬間」才印，不是每一圈都印
  if (motion != lastMotion) {
    lastMotion = motion;
    if (!warmedUp) {
      Serial.println("（暖機中，忽略）");
    } else if (motion) {
      Serial.println("偵測到有人");
    } else {
      Serial.println("恢復無人");
    }
  }

  delay(50);
}

/*
 * 動手試試
 *   等暖機一分鐘，在感測器前揮手，看有沒有出現「偵測到有人」。
 *   停住不動，等幾秒，看它什麼時候變回「恢復無人」。
 *
 * 為什麼要判斷「狀態改變」
 *   PIR 觸發後會維持 HIGH 好幾秒。
 *   如果每一圈都印，序列埠會被洗版；之後發手機通知，也會一直響個不停。
 *   這個「只在變化時動作」的寫法，在第六章的通知會再用一次。
 *
 * 常見狀況
 *   剛插電一直亂跳 → 還在暖機，等一分鐘。
 *   一直是有人 → 延遲旋鈕轉太大，或旁邊有熱源（暖氣、陽光直射）。
 */
