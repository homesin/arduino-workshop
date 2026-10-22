/*
 * U17 · SG90 伺服馬達
 *
 * 目標：角度控制，以及「供電」這件事。
 * 函式庫：Servo（Arduino IDE 內建，不用另外裝）
 *
 * 接線（SG90 三條線）：
 *   棕色 -- GND
 *   紅色 -- 5V
 *   橘色（訊號）-- D9
 *
 * 可變電阻同 U07：中間腳 -- A0，兩側分別接 5V 與 GND
 */

#include <Servo.h>

const int SERVO_PIN = 9;
const int POT_PIN = A0;

Servo servo;

void setup() {
  servo.attach(SERVO_PIN);
  Serial.begin(9600);

  // 先自己掃一圈，確認機構沒有卡住
  for (int angle = 0; angle <= 180; angle += 2) { servo.write(angle); delay(15); }
  for (int angle = 180; angle >= 0; angle -= 2) { servo.write(angle); delay(15); }
  delay(500);
  Serial.println("轉動可變電阻控制角度");
}

void loop() {
  int raw = analogRead(POT_PIN);
  int angle = map(raw, 0, 1023, 0, 180);
  servo.write(angle);

  Serial.print("角度 ");
  Serial.println(angle);
  delay(50);   // 給馬達一點時間轉過去
}

/*
 * 供電：這是伺服馬達最常見的問題
 *   SG90 空載大約 100~200 mA，堵轉瞬間可以到 700 mA。
 *   一顆用 Uno 的 5V 通常還撐得住，但會看到板子偶爾自己重開
 *   —— 那是電壓被拉低造成晶片重置。
 *   接兩顆以上、或馬達要出力，就要用獨立電源，
 *   而且獨立電源的 GND 一定要和 Uno 的 GND 接在一起。
 *
 * 抖動不停
 *   SG90 到定位後仍會微微修正。不需要持續控制時，
 *   可以用 servo.detach() 讓它放鬆。
 *
 * Servo 函式庫會占用 Timer1，
 * 所以用了它之後，D9 和 D10 的 analogWrite 會失效。
 */
