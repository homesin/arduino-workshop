/*
 * U16 · 七段顯示器 + 74HC595 移位暫存器
 *
 * 目標：用 3 支腳控制 8 個輸出。
 *
 * 七段顯示器有 8 個段（a~g 加小數點），直接接要用掉 8 支腳。
 * 74HC595 讓你用 3 支腳把 8 個位元「一位一位推進去」，再一次輸出。
 *
 * 接線（74HC595 是 16 腳 DIP，缺口朝上，左上為第 1 腳）：
 *   pin 8  GND      -- GND
 *   pin 16 VCC      -- 5V
 *   pin 10 MR       -- 5V    （高電位 = 不重置）
 *   pin 13 OE       -- GND   （低電位 = 輸出致能）
 *   pin 14 DS   資料 -- D4
 *   pin 12 STCP 鎖存 -- D5
 *   pin 11 SHCP 時脈 -- D6
 *
 *   Q0(pin 15) -[220R]- 七段 a       Q4(pin 4) -[220R]- 七段 e
 *   Q1(pin 1)  -[220R]- 七段 b       Q5(pin 5) -[220R]- 七段 f
 *   Q2(pin 2)  -[220R]- 七段 c       Q6(pin 6) -[220R]- 七段 g
 *   Q3(pin 3)  -[220R]- 七段 d       Q7(pin 7) -[220R]- 七段 dp
 *
 *   七段的共同腳（共陰）-- GND
 */

const int PIN_DATA  = 4;   // DS
const int PIN_LATCH = 5;   // STCP
const int PIN_CLOCK = 6;   // SHCP

// 每個位元對應一段：bit0=a bit1=b bit2=c bit3=d bit4=e bit5=f bit6=g bit7=dp
// 共陰接法：該位元為 1 就亮
const byte DIGITS[10] = {
  0b00111111,  // 0
  0b00000110,  // 1
  0b01011011,  // 2
  0b01001111,  // 3
  0b01100110,  // 4
  0b01101101,  // 5
  0b01111101,  // 6
  0b00000111,  // 7
  0b01111111,  // 8
  0b01101111   // 9
};

void showByte(byte pattern) {
  digitalWrite(PIN_LATCH, LOW);                       // 開始推資料
  shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, pattern);   // 一次推 8 個位元
  digitalWrite(PIN_LATCH, HIGH);                      // 鎖存 → 8 個輸出同時改變
}

void showDigit(int n, bool dot) {
  byte pattern = DIGITS[constrain(n, 0, 9)];
  if (dot) pattern |= 0b10000000;
  showByte(pattern);
}

void setup() {
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
}

void loop() {
  // 9 到 0 倒數
  for (int n = 9; n >= 0; n--) {
    showDigit(n, false);
    delay(600);
  }

  // 全部熄滅一下，再把小數點單獨點亮
  showByte(0b00000000);  delay(300);
  showByte(0b10000000);  delay(600);
}

/*
 * 移位暫存器在做什麼
 *   SHCP 每給一個脈衝，就把 DS 上的一個位元推進暫存器，
 *   原本的位元往旁邊移一格。推滿 8 個之後，
 *   STCP 給一個脈衝，8 個輸出才同時更新 —— 所以畫面不會閃。
 *
 * 如果數字是亂的
 *   最常見是段的順序接錯。先只點亮一段測試：
 *   showByte(0b00000001) 應該只亮 a 段（最上面那一橫）。
 *   照著 a b c d e f g dp 的順序一段一段確認。
 *
 * 如果整個是反的（該亮的不亮、該暗的全亮）
 *   買到共陽版本了。把共同腳改接 5V，
 *   並在 showByte 裡把 pattern 取反：shiftOut(..., ~pattern)。
 */
