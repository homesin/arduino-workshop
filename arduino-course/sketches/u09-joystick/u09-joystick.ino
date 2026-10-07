/*
 * U09 · 搖桿：兩個類比軸加一個按鍵
 *
 * 目標：同時處理多個類比輸入，並認識「靜止區」。
 * 接線：
 *   GND ── GND      +5V ── 5V
 *   VRx ── A0       VRy ── A1       SW ── D2
 *
 * 搖桿的 SW 按下時接到 GND，所以和按鈕一樣要用 INPUT_PULLUP。
 */

const int PIN_X = A0;
const int PIN_Y = A1;
const int PIN_SW = 2;

const int CENTER = 512;       // 置中時的理論值
const int DEADZONE = 60;      // 靜止區：手不碰時讀值也會飄，這範圍內一律當作沒動

void setup() {
  pinMode(PIN_SW, INPUT_PULLUP);
  Serial.begin(9600);
}

// 把 0~1023 換算成 -100 ~ +100，中間有靜止區
int axisPercent(int raw) {
  int offset = raw - CENTER;
  if (abs(offset) < DEADZONE) return 0;
  return constrain(map(offset, -CENTER, CENTER, -100, 100), -100, 100);
}

void loop() {
  int x = axisPercent(analogRead(PIN_X));
  int y = axisPercent(analogRead(PIN_Y));
  bool pressed = digitalRead(PIN_SW) == LOW;

  Serial.print("X ");   Serial.print(x);
  Serial.print("\tY "); Serial.print(y);
  Serial.print("\t");
  if (pressed)      Serial.print("按下");
  else if (y > 40)  Serial.print("上");
  else if (y < -40) Serial.print("下");
  else if (x > 40)  Serial.print("右");
  else if (x < -40) Serial.print("左");
  else              Serial.print("置中");
  Serial.println();

  delay(120);
}

/*
 * 為什麼需要靜止區
 *   把 DEADZONE 改成 0 再看序列埠：手完全沒碰，數字也會一直跳。
 *   類比讀值本來就有雜訊，靜止區是最簡單的處理方式。
 */
