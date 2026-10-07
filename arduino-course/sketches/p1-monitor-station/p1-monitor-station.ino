/*
 * 整合專題 · 校園環境監測站
 *
 * 把整套課程用過的東西組成一台真的能放著跑的裝置：
 *   量測  DHT11 溫溼度、超音波距離、火焰、光線、水位
 *   顯示  七段顯示器顯示狀態碼、雙色 LED 顯示警戒等級
 *   反應  蜂鳴器本地警報、伺服馬達舉起警示旗
 *   回報  每秒送一行 JSON 給電腦，由 Node.js 分流到雲端與 LINE
 *   受控  電腦可以下指令調整門檻、消音、測試警報
 *
 * ── 腳位表 ──────────────────────────────────────────
 *   D2   DHT11 DATA
 *   D3   HC-SR501 PIR OUT
 *   D4   HC-SR04 Trig
 *   D5   HC-SR04 Echo
 *   D6   無源蜂鳴器
 *   D7   紅色 LED  -[220R]-      警戒
 *   D8   綠色 LED  -[220R]-      正常
 *   D9   SG90 伺服馬達 訊號
 *   D10  74HC595 DS   (pin 14)
 *   D11  74HC595 STCP (pin 12)
 *   D12  74HC595 SHCP (pin 11)
 *   A0   火焰感測 AO
 *   A1   光敏電阻分壓中點
 *   A2   水位感測 S
 *   A3   可變電阻中間腳（現場調火焰門檻）
 *
 *   74HC595 的 Q0~Q7 各串 220R 接七段 a~g 與 dp，七段共陰腳接 GND
 *
 * ── 需要的函式庫 ────────────────────────────────────
 *   SimpleDHT（要自行安裝）、Servo（IDE 內建）
 *
 * ── 注意 ────────────────────────────────────────────
 *   Servo 占用 Timer1，D9 D10 的 analogWrite 失效（這裡用不到）
 *   tone() 占用 Timer2，D3 D11 的 analogWrite 失效（這裡也用不到）
 */

#include <SimpleDHT.h>
#include <Servo.h>

// ── 腳位 ──
const int PIN_DHT    = 2;
const int PIN_PIR    = 3;
const int PIN_TRIG   = 4;
const int PIN_ECHO   = 5;
const int PIN_BUZZER = 6;
const int PIN_LED_R  = 7;
const int PIN_LED_G  = 8;
const int PIN_SERVO  = 9;
const int PIN_595_DS   = 10;
const int PIN_595_LATCH= 11;
const int PIN_595_CLK  = 12;
const int PIN_FLAME = A0;
const int PIN_LIGHT = A1;
const int PIN_WATER = A2;
const int PIN_POT   = A3;

// ── 門檻（開機後可由可變電阻或電腦指令調整）──
int flameThreshold = 400;    // 低於此值 = 有火
int waterThreshold = 300;    // 高於此值 = 有水
const int DARK_THRESHOLD = 350;
const float NEAR_CM = 15.0;  // 近於此距離 = 有東西擋住

// ── 狀態碼（顯示在七段上）──
const int ST_OK      = 0;
const int ST_PERSON  = 1;
const int ST_NEAR    = 2;
const int ST_WATER   = 3;
const int ST_FIRE    = 4;

// ── 七段字型：bit0=a ... bit6=g, bit7=dp（共陰）──
const byte DIGITS[10] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
  0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111
};

SimpleDHT11 dht11(PIN_DHT);
Servo flagServo;

byte lastTemp = 0, lastHum = 0;
bool hasDht = false;

unsigned long lastSendAt = 0;
unsigned long lastSensorAt = 0;
unsigned long lastBeepAt = 0;
bool muted = false;

const unsigned long SEND_PERIOD = 1000;
const unsigned long SENSOR_PERIOD = 200;
const unsigned long BEEP_PERIOD = 800;

// 最近一次的量測結果
float distanceCm = -1;
int flameRaw = 1023, lightRaw = 0, waterRaw = 0;
bool personSeen = false;
int status = ST_OK;

// ── 七段 ──
void showDigit(int n) {
  byte pattern = DIGITS[constrain(n, 0, 9)];
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DS, PIN_595_CLK, LSBFIRST, pattern);
  digitalWrite(PIN_595_LATCH, HIGH);
}

// ── 超音波 ──
float readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long d = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (d == 0) return -1;
  return d * 0.0343f / 2.0f;
}

// ── 判斷整體狀態：越後面的優先級越高 ──
int evaluateStatus() {
  if (flameRaw < flameThreshold) return ST_FIRE;
  if (waterRaw > waterThreshold) return ST_WATER;
  if (distanceCm > 0 && distanceCm < NEAR_CM) return ST_NEAR;
  if (personSeen) return ST_PERSON;
  return ST_OK;
}

// ── 送一行 JSON 給電腦 ──
void sendReading() {
  Serial.print("{\"t\":");      Serial.print(lastTemp);
  Serial.print(",\"h\":");      Serial.print(lastHum);
  Serial.print(",\"pir\":");    Serial.print(personSeen ? 1 : 0);
  Serial.print(",\"flame\":");  Serial.print(flameRaw);
  Serial.print(",\"light\":");  Serial.print(lightRaw);
  Serial.print(",\"water\":");  Serial.print(waterRaw);
  Serial.print(",\"dist\":");   Serial.print(distanceCm, 1);
  Serial.print(",\"status\":"); Serial.print(status);
  Serial.println("}");
}

// ── 處理電腦下來的指令 ──
void handleCommand(String line) {
  line.trim();
  if (line.length() == 0) return;

  int sep = line.indexOf(':');
  if (sep < 0) { Serial.println("# 格式應為 名稱:數值"); return; }

  String name = line.substring(0, sep);
  int value = line.substring(sep + 1).toInt();

  if (name == "FLAME") {
    flameThreshold = constrain(value, 0, 1023);
    Serial.print("# 火焰門檻 = "); Serial.println(flameThreshold);
  } else if (name == "WATER") {
    waterThreshold = constrain(value, 0, 1023);
    Serial.print("# 水位門檻 = "); Serial.println(waterThreshold);
  } else if (name == "MUTE") {
    muted = value > 0;
    if (muted) noTone(PIN_BUZZER);
    Serial.print("# 消音 = "); Serial.println(muted ? "開" : "關");
  } else if (name == "TEST") {
    Serial.println("# 測試警報");
    tone(PIN_BUZZER, 880, 200);
    flagServo.write(90); delay(400); flagServo.write(0);
  } else {
    Serial.print("# 不認識的指令 "); Serial.println(name);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_595_DS, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);
  pinMode(PIN_595_CLK, OUTPUT);

  flagServo.attach(PIN_SERVO);
  flagServo.write(0);            // 旗子放下

  showDigit(8);                  // 開機自我測試：七段全亮
  digitalWrite(PIN_LED_R, HIGH);
  digitalWrite(PIN_LED_G, HIGH);
  tone(PIN_BUZZER, 660, 120);
  delay(600);
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);

  Serial.println("# monitor station ready");
  Serial.println("# 可用指令 FLAME:400 WATER:300 MUTE:1 TEST:1");
}

void loop() {
  unsigned long now = millis();

  // 1) 隨時接收電腦指令
  if (Serial.available()) handleCommand(Serial.readStringUntil('\n'));

  // 2) 每 200 毫秒量一次感測器
  if (now - lastSensorAt >= SENSOR_PERIOD) {
    lastSensorAt = now;

    flameRaw = analogRead(PIN_FLAME);
    lightRaw = analogRead(PIN_LIGHT);
    waterRaw = analogRead(PIN_WATER);
    personSeen = digitalRead(PIN_PIR) == HIGH;
    distanceCm = readDistanceCm();

    // 可變電阻即時微調火焰門檻，現場不必重新燒錄
    flameThreshold = map(analogRead(PIN_POT), 0, 1023, 100, 900);

    status = evaluateStatus();

    // 顯示與指示燈
    showDigit(status);
    digitalWrite(PIN_LED_G, status == ST_OK ? HIGH : LOW);
    digitalWrite(PIN_LED_R, status == ST_OK ? LOW : HIGH);

    // 警示旗：火警或淹水才舉起來
    flagServo.write((status == ST_FIRE || status == ST_WATER) ? 90 : 0);
  }

  // 3) 本地警報：用 millis 控制節奏，不擋住其他工作
  if (!muted && status != ST_OK && now - lastBeepAt >= BEEP_PERIOD) {
    lastBeepAt = now;
    // 越嚴重叫得越高、越急
    if (status == ST_FIRE)       tone(PIN_BUZZER, 1200, 300);
    else if (status == ST_WATER) tone(PIN_BUZZER, 900, 250);
    else                         tone(PIN_BUZZER, 660, 100);
  }

  // 4) 每秒回報一次給電腦
  if (now - lastSendAt >= SEND_PERIOD) {
    lastSendAt = now;
    byte t = 0, h = 0;
    if (dht11.read(&t, &h, NULL) == SimpleDHTErrSuccess) {
      lastTemp = t; lastHum = h; hasDht = true;
    }
    if (hasDht) sendReading();
  }
}

/*
 * 狀態碼對照（顯示在七段上）
 *   0  正常
 *   1  偵測到有人（PIR）
 *   2  有東西靠近（超音波 15 公分內）
 *   3  淹水
 *   4  火警
 *
 * 為什麼整支程式沒有一個 delay()
 *   因為它要同時做四件事：收指令、量感測器、發警報聲、回報資料。
 *   四件事的節奏都不一樣，只要用了 delay 就會互相卡住。
 *   這就是 U11 那一課的實際用途。
 *
 * 現場調校順序
 *   1. 先看序列埠印出的 flame / light / water 原始值
 *   2. 轉可變電阻，讓火焰門檻落在「平常」與「打火機靠近」中間
 *   3. 水位門檻用指令調：WATER:350
 *   4. 都調好之後再接 Node.js
 */
