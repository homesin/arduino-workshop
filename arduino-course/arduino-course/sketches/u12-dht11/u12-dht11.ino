/*
 * U12 · DHT11 溫溼度感測
 *
 * 目標：第一次用外部函式庫。
 * 函式庫：工具 → 管理程式庫 → 搜尋 SimpleDHT（作者 Winlin）→ 安裝
 *
 * 接線（Circus DHT11 模組，三支排針）：
 *   VCC ── 5V     GND ── GND     DATA ── D2
 */

#include <SimpleDHT.h>

const int DHT_PIN = 2;
SimpleDHT11 dht11(DHT_PIN);

byte lastTemp = 0;
byte lastHum = 0;
bool hasReading = false;

void setup() {
  Serial.begin(9600);
  Serial.println("DHT11 開始讀取（每秒一次）");
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;
  int err = dht11.read(&temperature, &humidity, NULL);

  if (err == SimpleDHTErrSuccess) {
    lastTemp = temperature;
    lastHum = humidity;
    hasReading = true;
  } else {
    Serial.print("讀取失敗（錯誤碼 ");
    Serial.print(err);
    Serial.println("），沿用上一次的值");
  }

  if (hasReading) {
    Serial.print("溫度 ");
    Serial.print(lastTemp);
    Serial.print(" °C    濕度 ");
    Serial.print(lastHum);
    Serial.println(" %");
  }

  delay(1500);   // DHT11 官方建議至少 1 秒一次，讀太快一定失敗
}

/*
 * DHT11 的脾氣
 *   反應慢：溫度變化要好幾秒才跟得上
 *   精度低：溫度 ±2°C、濕度 ±5%，而且只到整數
 *   讀太快就會失敗，所以要保留上一次的值
 *
 * 想要準一點就換 DHT22，接線和程式幾乎一樣。
 *
 * 動手試試
 *   對著感測器呵一口氣，看濕度衝上去再慢慢回落。
 */
