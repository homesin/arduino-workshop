/*
 * U21 · 設計一個序列埠協定
 *
 * 目標：讓 Arduino 和電腦講同一種話。
 *
 * 到目前為止，Serial.print 只是印給人看的。
 * 一旦要讓電腦上的程式讀，就必須先講好格式 —— 那就是「協定」。
 *
 * 這裡用的規則只有三條：
 *   1. 一行一筆資料，用換行結尾
 *   2. 資料一律是 JSON 物件，方便電腦解析
 *   3. 不是資料的訊息（開機提示、錯誤）以 # 開頭，電腦看到就略過
 *
 * 反過來，電腦也可以送指令下來，格式是「名稱:數值」。
 *
 * 接線：
 *   DHT11 DATA -- D2
 *   PIR   OUT  -- D3
 *   LED   -[220R]- D9      （電腦可以控制亮度）
 *   蜂鳴器      -- D8      （電腦可以叫它響）
 *   火焰 AO     -- A0
 */

#include <SimpleDHT.h>

const int PIN_DHT = 2;
const int PIN_PIR = 3;
const int PIN_LED = 9;
const int PIN_BUZZER = 8;
const int PIN_FLAME = A0;

SimpleDHT11 dht11(PIN_DHT);

byte lastTemp = 0, lastHum = 0;
bool hasReading = false;

unsigned long lastSendAt = 0;
const unsigned long SEND_PERIOD = 1000;   // 每秒送一筆

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  Serial.println("# sensor node ready");   // # 開頭 = 給人看的，電腦會略過
}

// 送出一行 JSON
void sendReading() {
  Serial.print("{\"t\":");     Serial.print(lastTemp);
  Serial.print(",\"h\":");     Serial.print(lastHum);
  Serial.print(",\"pir\":");   Serial.print(digitalRead(PIN_PIR) == HIGH ? 1 : 0);
  Serial.print(",\"flame\":"); Serial.print(analogRead(PIN_FLAME));
  Serial.println("}");
}

// 處理電腦送下來的一行指令
void handleCommand(String line) {
  line.trim();
  if (line.length() == 0) return;

  int sep = line.indexOf(':');
  if (sep < 0) {
    Serial.println("# 指令格式錯誤，應為 名稱:數值");
    return;
  }

  String name = line.substring(0, sep);
  int value = line.substring(sep + 1).toInt();

  if (name == "LED") {
    analogWrite(PIN_LED, constrain(value, 0, 255));
    Serial.print("# LED 亮度設為 "); Serial.println(value);
  } else if (name == "BUZZ") {
    if (value > 0) tone(PIN_BUZZER, 880, value);   // 數值當作持續毫秒數
    else noTone(PIN_BUZZER);
    Serial.print("# 蜂鳴器 "); Serial.println(value);
  } else {
    Serial.print("# 不認識的指令 "); Serial.println(name);
  }
}

void loop() {
  // 有指令進來就先處理（不會卡住，沒東西時 available() 是 0）
  if (Serial.available()) {
    handleCommand(Serial.readStringUntil('\n'));
  }

  // 每秒送一筆，用 millis 不用 delay，才不會擋住上面的指令處理
  unsigned long now = millis();
  if (now - lastSendAt >= SEND_PERIOD) {
    lastSendAt = now;

    byte t = 0, h = 0;
    if (dht11.read(&t, &h, NULL) == SimpleDHTErrSuccess) {
      lastTemp = t; lastHum = h; hasReading = true;
    }
    if (hasReading) sendReading();
  }
}

/*
 * 自己測試協定
 *   開序列埠監控視窗，右下角把「沒有行結尾」改成「新行」，
 *   然後在輸入框打：
 *      LED:255      -> 燈全亮
 *      LED:20       -> 燈微亮
 *      BUZZ:300     -> 響 0.3 秒
 *
 * 為什麼要用 JSON 而不是 "26,58,0,812"
 *   逗號分隔看起來比較短，但少一個欄位、順序換掉，
 *   電腦那端就整個錯位而且不會報錯。
 *   JSON 有欄位名稱，加減欄位都不會壞掉。
 *
 * 為什麼非資料訊息要用 # 開頭
 *   開機提示、錯誤訊息如果直接混進資料流，
 *   電腦那端解析 JSON 就會失敗。給它一個好認的前綴，
 *   電腦看到就跳過，人看序列埠監控視窗時又讀得懂。
 */
