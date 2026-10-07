/*
 * U05 · RGB LED 混色
 *
 * 目標：三路 PWM 各自控制，混出任意顏色。
 * 接線（10mm 共陰 RGB LED，最長那支是共同腳）：
 *   R 腳 ──[220Ω]── D9
 *   共陰腳 ── GND
 *   G 腳 ──[220Ω]── D10
 *   B 腳 ──[220Ω]── D11
 *
 * 共陰 = 共同腳接 GND，某一色給越大的值就越亮。
 * 若買到共陽版本，共同腳改接 5V，數值要反過來寫（255 - 值）。
 */

const int PIN_R = 9;
const int PIN_G = 10;
const int PIN_B = 11;

void setColor(int r, int g, int b) {
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void setup() {
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
}

void loop() {
  setColor(255,   0,   0);  delay(600);   // 紅
  setColor(  0, 255,   0);  delay(600);   // 綠
  setColor(  0,   0, 255);  delay(600);   // 藍
  setColor(255, 180,   0);  delay(600);   // 橘（紅多綠少）
  setColor(160,   0, 255);  delay(600);   // 紫（紅藍相加）
  setColor(255, 255, 255);  delay(600);   // 白（三色全開）

  // 沿著色相環平滑轉一圈
  for (int hue = 0; hue < 256; hue++) {
    // 三個相位各差 1/3 圈，用三角波近似色相環
    int r = abs(((hue      ) % 256) - 128) * 2 - 128;
    int g = abs(((hue +  85) % 256) - 128) * 2 - 128;
    int b = abs(((hue + 170) % 256) - 128) * 2 - 128;
    setColor(constrain(r, 0, 255), constrain(g, 0, 255), constrain(b, 0, 255));
    delay(15);
  }
}
