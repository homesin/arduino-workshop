/*
 * U10 · 無源蜂鳴器：讓板子發出聲音
 *
 * 目標：tone() 控制頻率，做出音階與警報聲。
 * 接線：
 *   蜂鳴器 正極(長腳或標 +) ── D8
 *   蜂鳴器 另一腳            ── GND
 *
 * 無源蜂鳴器需要你給它「頻率」才會響；
 * 有源蜂鳴器只要通電就叫，反而不能控制音高。
 */

const int BUZZER_PIN = 8;

// 中央 Do 到高音 Do 的頻率（赫茲）
const int SCALE[] = {262, 294, 330, 349, 392, 440, 494, 523};
const char* NAMES[] = {"Do", "Re", "Mi", "Fa", "So", "La", "Si", "Do"};

void setup() {
  Serial.begin(9600);

  // 先跑一次音階
  for (int i = 0; i < 8; i++) {
    Serial.println(NAMES[i]);
    tone(BUZZER_PIN, SCALE[i], 300);   // 第三個參數是持續毫秒數
    delay(350);                        // 要比 tone 的長度再久一點才聽得出斷句
  }
  noTone(BUZZER_PIN);
  delay(800);
}

void loop() {
  // 兩音交替的警報聲
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 880); delay(200);
    tone(BUZZER_PIN, 660); delay(200);
  }
  noTone(BUZZER_PIN);
  delay(2000);
}

/*
 * 注意
 *   tone() 在 Uno 上會占用 Timer2，
 *   所以用了 tone() 之後，D3 和 D11 的 analogWrite 會失效。
 *   要同時做呼吸燈和聲音的話，LED 改用 D5 D6 D9 D10。
 */
