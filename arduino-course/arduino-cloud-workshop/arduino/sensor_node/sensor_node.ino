/*
 * 校園環境監測站 — Arduino 端
 *
 * 每秒往 USB 送一行 JSON，電腦上的 Node.js 負責接收與分流：
 *   {"t":25.3,"h":60,"pir":0,"flame":812}
 *
 * 需要的函式庫（Arduino IDE → 工具 → 管理程式庫，搜尋安裝）：
 *   SimpleDHT   by Winlin      ← 讀 DHT11，零相依套件
 *
 * 接線（全部用母/母杜邦線直接插 Uno，這一段不需要麵包板）：
 *   DHT11 模組     VCC→5V    GND→GND   DATA→D2
 *   HC-SR501 PIR   VCC→5V    GND→GND   OUT →D3
 *   火焰感測模組    VCC→5V    GND→GND   AO  →A0
 *
 * 上一堂的 LED 留在 D9 也沒關係，兩者不衝突。
 */

#include <SimpleDHT.h>

const int PIN_DHT   = 2;
const int PIN_PIR   = 3;
const int PIN_FLAME = A0;

SimpleDHT11 dht11(PIN_DHT);

// DHT11 反應慢，讀太快會失敗；記住上一次的有效值，讀失敗就沿用
byte lastTemp = 0;
byte lastHum  = 0;
bool hasReading = false;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PIR, INPUT);
  // 這一行不是 JSON，Node.js 端會自動略過，純粹讓你在序列埠監控看到板子活著
  Serial.println("# sensor node ready");
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;

  // 讀成功才更新，失敗就沿用上一次，避免畫面一直跳 0
  if (dht11.read(&temperature, &humidity, NULL) == SimpleDHTErrSuccess) {
    lastTemp = temperature;
    lastHum = humidity;
    hasReading = true;
  }

  int pir = digitalRead(PIN_PIR) == HIGH ? 1 : 0;
  int flame = analogRead(PIN_FLAME);

  // 還沒讀到任何一次溫濕度就先不送，讓 Node.js 端不用處理空值
  if (hasReading) {
    Serial.print("{\"t\":");
    Serial.print(lastTemp);
    Serial.print(",\"h\":");
    Serial.print(lastHum);
    Serial.print(",\"pir\":");
    Serial.print(pir);
    Serial.print(",\"flame\":");
    Serial.print(flame);
    Serial.println("}");
  }

  delay(1000);   // DHT11 官方建議至少 1 秒一次
}
