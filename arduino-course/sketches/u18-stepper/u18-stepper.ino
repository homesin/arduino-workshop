/*
 * U18 · 28BYJ-48 步進馬達
 *
 * 目標：精準轉幾度，而不是「轉一下」。
 * 函式庫：Stepper（Arduino IDE 內建）
 *
 * 接線（ULN2003 驅動板）：
 *   IN1 -- D8    IN2 -- D9    IN3 -- D10    IN4 -- D11
 *   驅動板 5V -- 5V      GND -- GND
 *   馬達的白色接頭插上驅動板即可（只有一個方向插得進去）
 */

#include <Stepper.h>

// 28BYJ-48 內部有減速齒輪，輸出軸轉一圈約 2048 步（全步進）
const int STEPS_PER_REV = 2048;

// 注意腳位順序是 IN1, IN3, IN2, IN4
// 這是 Stepper 函式庫的激磁順序要求，寫成 8,9,10,11 只會抖動不轉
Stepper motor(STEPS_PER_REV, 8, 10, 9, 11);

void setup() {
  motor.setSpeed(10);        // 每分鐘 10 轉；28BYJ-48 最高約 15
  Serial.begin(9600);
}

void loop() {
  Serial.println("順時針一整圈");
  motor.step(STEPS_PER_REV);
  delay(800);

  Serial.println("逆時針半圈");
  motor.step(-STEPS_PER_REV / 2);
  delay(800);

  Serial.println("順時針 90 度");
  motor.step(STEPS_PER_REV / 4);
  delay(1500);
}

/*
 * 步進馬達和伺服馬達差在哪
 *   伺服：你說「轉到 90 度」，它自己知道現在在哪，會轉過去
 *   步進：你說「走 512 步」，它照做，但它不知道自己在哪
 *         —— 開機時停的位置就是它認定的原點
 *
 *   所以步進馬達適合「要轉很多圈」或「要很細的角度」，
 *   伺服馬達適合「要固定在某個角度」。
 *
 * 只抖動不轉動
 *   九成是腳位順序寫錯。記得是 IN1, IN3, IN2, IN4。
 *
 * 轉起來很燙
 *   步進馬達停著的時候仍然在通電維持扭力，這是正常的。
 *   不需要保持位置時，可以把四支腳都寫成 LOW 讓它斷電。
 *
 * motor.step() 會卡住整個程式直到走完。
 * 需要邊轉邊做別的事，要改用 AccelStepper 這類非阻塞的函式庫。
 */
