/*
 * 第五章 · 感測資料整合：校園環境監測站（Arduino 端）
 *
 * 目標：四個感測器一起讀，不用 delay（改用 millis），整理成一行 JSON 送給電腦。
 *
 * 每秒往 USB 送一行 JSON，電腦上的 Node.js 負責接收與分流：
 *   {"t":25,"h":60,"pir":0,"dist":23,"light":280}
 *
 * 需要的函式庫（Arduino IDE → 工具 → 管理程式庫，搜尋安裝）：
 *   SimpleDHT   by Winlin      ← 讀 DHT11，零相依套件
 *
 * 接線（四個感測器VCC 接麵包板紅軌（5V）、GND 接藍軌，訊號腳直接接 Uno）：
 *   DHT11 模組        VCC→紅軌  GND→藍軌  DATA→D2
 *   HC-SR501 PIR      VCC→紅軌  GND→藍軌  OUT →D3
 *   HC-SR04 超音波    VCC→紅軌  GND→藍軌  Trig→D4   Echo→D5
 *   LM393 光敏模組    VCC→紅軌  GND→藍軌  AO  →A1   （DO 不接）
 *
 * 注意：LM393 光敏模組是「越暗數值越大」。
 */

#include <SimpleDHT.h>

const int PIN_DHT   = 2;
const int PIN_PIR   = 3;
const int PIN_TRIG  = 4;
const int PIN_ECHO  = 5;
const int PIN_LIGHT = A1;

SimpleDHT11 dht11(PIN_DHT);

// DHT11 反應慢，讀太快會失敗；記住上一次的有效值，讀失敗就沿用
byte lastTemp = 0;
byte lastHum  = 0;
bool hasReading = false;

unsigned long lastDht = 0;
unsigned long lastSend = 0;

// 回傳公分；沒收到回波（超出量程或沒接好）回傳 -1
int readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000UL);   // 30 ms 約等於 5 公尺，逾時就放棄
  return us ? (int)(us / 58) : -1;
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  // 這一行不是 JSON，Node.js 端會自動略過，純粹讓你在序列埠監控看到板子活著
  Serial.println("# sensor node ready");
}

void loop() {
  unsigned long now = millis();

  // DHT11 官方建議至少 1 秒讀一次，這裡每 2 秒讀一次，其餘時間沿用上一次的值
  if (now - lastDht >= 2000 || lastDht == 0) {
    lastDht = now ? now : 1;
    byte temperature = 0;
    byte humidity = 0;
    if (dht11.read(&temperature, &humidity, NULL) == SimpleDHTErrSuccess) {
      lastTemp = temperature;
      lastHum = humidity;
      hasReading = true;
    }
  }

  if (now - lastSend >= 1000) {
    lastSend = now;
    int pir = digitalRead(PIN_PIR) == HIGH ? 1 : 0;
    int dist = readDistanceCm();
    int light = analogRead(PIN_LIGHT);

    // 還沒讀到任何一次溫濕度就先不送，讓 Node.js 端不用處理空值
    if (hasReading) {
      Serial.print("{\"t\":");
      Serial.print(lastTemp);
      Serial.print(",\"h\":");
      Serial.print(lastHum);
      Serial.print(",\"pir\":");
      Serial.print(pir);
      Serial.print(",\"dist\":");
      Serial.print(dist);
      Serial.print(",\"light\":");
      Serial.print(light);
      Serial.println("}");
    }
  }
}
