/*
 * 第四章 · 感測器 1：DHT11 溫濕度
 *
 * 目標：第一次用外部函式庫，並學會處理「讀取失敗」。
 * 函式庫：工具 → 管理程式庫 → 搜尋 SimpleDHT（作者 Winlin）→ 安裝
 *        （install.cmd 已經幫你裝好了）
 *
 * 接線（三支針腳）：
 *   VCC ── 5V     GND ── GND     DATA ── D2
 *
 * 原理：DHT11 內部有一個濕度元件和一個溫度元件，晶片把量到的值用「一條線」送出來，
 *       函式庫負責解讀，我們只要呼叫 read()。
 */

#include <SimpleDHT.h>

const int DHT_PIN = 2;
const int TOO_HOT = 32;           // 判斷門檻：超過幾度算太熱（先看室溫，再依現場調整）

SimpleDHT11 dht11(DHT_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("DHT11 開始讀取（每 2 秒一次）");
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;

  // read() 回傳錯誤碼：成功是 SimpleDHTErrSuccess，其他都是失敗
  int err = dht11.read(&temperature, &humidity, NULL);

  if (err != SimpleDHTErrSuccess) {
    Serial.print("讀取失敗，錯誤碼 ");
    Serial.println(err);
  } else {
    Serial.print("溫度 ");
    Serial.print(temperature);
    Serial.print(" °C    濕度 ");
    Serial.print(humidity);
    Serial.print(" %");
    if (temperature > TOO_HOT) {
      Serial.print("    ← 太熱了");
    }
    Serial.println();
  }

  delay(2000);                    // DHT11 讀太快一定失敗，至少間隔 1 秒
}

/*
 * 動手試試
 *   對著感測器呵一口氣，看濕度衝上去，再慢慢回落。
 *   用手握住感測器，看溫度慢慢升高（DHT11 反應很慢，要等幾秒），超過門檻會出現「太熱了」。
 *   室溫本來就超過門檻的話，把 TOO_HOT 調高再上傳。
 *
 * DHT11 的脾氣
 *   精度低：溫度 ±2°C、濕度 ±5%，而且只到整數。
 *   讀太快會失敗，所以 delay 不能太小。
 *   一直讀到失敗：先檢查 DATA 有沒有接在 D2、VCC 與 GND 有沒有插好。
 */
