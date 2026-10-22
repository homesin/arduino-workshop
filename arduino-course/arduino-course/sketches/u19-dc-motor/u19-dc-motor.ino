/*
 * U19 · L9110 直流馬達驅動
 *
 * 目標：正反轉與調速，以及「為什麼不能直接接腳位」。
 *
 * Arduino 每支腳最多只能出 20 mA，馬達動輒幾百 mA，
 * 直接接會燒掉腳位。驅動模組的工作就是：
 * 用小電流的訊號，去控制大電流的通路。
 *
 * 接線（L9110 單板）：
 *   VCC -- 5V      GND -- GND
 *   A-IA -- D5     A-IB -- D6      （兩支都要是 PWM 腳）
 *   馬達兩條線接到 MOTOR A 的兩個端子（正反不拘，反了就是轉向相反）
 */

const int PIN_IA = 5;
const int PIN_IB = 6;

// speed 範圍 -255 ~ +255：正值正轉、負值反轉、0 停止
void drive(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) {
    analogWrite(PIN_IA, speed);
    analogWrite(PIN_IB, 0);
  } else {
    analogWrite(PIN_IA, 0);
    analogWrite(PIN_IB, -speed);
  }
}

void setup() {
  pinMode(PIN_IA, OUTPUT);
  pinMode(PIN_IB, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.println("正轉，慢慢加速");
  for (int s = 80; s <= 255; s += 5) { drive(s); delay(60); }
  delay(800);

  Serial.println("停止");
  drive(0);
  delay(1000);

  Serial.println("反轉");
  drive(-200);
  delay(1500);

  Serial.println("停止");
  drive(0);
  delay(1500);
}

/*
 * L9110 怎麼決定方向
 *   兩支輸入腳的相對關係決定轉向：
 *     IA 高 / IB 低  -> 正轉
 *     IA 低 / IB 高  -> 反轉
 *     兩支都低        -> 自由停轉（靠慣性滑行）
 *     兩支都高        -> 短路煞停（急停，一直維持會發熱）
 *   給 PWM 而不是 HIGH，就是在調速。
 *
 * 為什麼從 80 開始加速而不是從 0
 *   直流馬達在低轉速時扭力不足，通常 60~90 以下只會嗡嗡叫不會動。
 *   這個「起動門檻」每顆馬達都不一樣，要自己試。
 *
 * 馬達一轉，板子就重開
 *   馬達啟動的瞬間電流很大，把 5V 拉垮了。
 *   改用獨立電源供應馬達，並把兩邊的 GND 接在一起。
 */
