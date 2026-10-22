/*
 * U20 · 4x4 矩陣鍵盤：做一個密碼鎖
 *
 * 目標：矩陣掃描的概念，以及把前面學的東西組起來。
 * 函式庫：工具 -> 管理程式庫 -> 搜尋 Keypad（作者 Mark Stanley）-> 安裝
 *
 * 16 顆按鍵如果一顆一支腳要 16 支，矩陣接法只要 8 支：
 * 排成 4 列 4 行，逐列送電、逐行讀取，就知道是哪一顆被按。
 *
 * 接線（薄膜鍵盤的排線，正面朝上，由左至右共 8 條）：
 *   第 1~4 條（列 R1~R4）-- D9 D8 D7 D6
 *   第 5~8 條（行 C1~C4）-- D5 D4 D3 D2
 *
 *   綠色 LED -[220R]- D10     （開鎖）
 *   紅色 LED -[220R]- D11     （錯誤）
 *   蜂鳴器 -- D12
 */

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int LED_OK = 10;
const int LED_NG = 11;
const int BUZZER = 12;

const char* PASSWORD = "1234";
const int MAX_LEN = 8;

char entered[MAX_LEN + 1] = "";
int enteredLen = 0;

void resetInput() {
  enteredLen = 0;
  entered[0] = '\0';
}

void beep(int freq, int ms) {
  tone(BUZZER, freq, ms);
  delay(ms + 30);
}

void setup() {
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_NG, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  Serial.println("請輸入密碼，# 送出，* 清除");
}

void loop() {
  char key = keypad.getKey();
  if (!key) return;              // 沒有人按，直接離開這一輪

  beep(1200, 40);

  if (key == '*') {
    resetInput();
    Serial.println("已清除");
    return;
  }

  if (key == '#') {
    if (strcmp(entered, PASSWORD) == 0) {
      Serial.println("密碼正確，開鎖");
      digitalWrite(LED_OK, HIGH);
      beep(880, 120);
      beep(1320, 200);
      delay(2000);
      digitalWrite(LED_OK, LOW);
    } else {
      Serial.println("密碼錯誤");
      digitalWrite(LED_NG, HIGH);
      beep(220, 400);
      delay(600);
      digitalWrite(LED_NG, LOW);
    }
    resetInput();
    return;
  }

  // 一般按鍵：存起來
  if (enteredLen < MAX_LEN) {
    entered[enteredLen++] = key;
    entered[enteredLen] = '\0';
    Serial.print("已輸入 ");
    for (int i = 0; i < enteredLen; i++) Serial.print('*');   // 不把密碼印出來
    Serial.println();
  } else {
    Serial.println("太長了，請按 * 清除");
  }
}

/*
 * 按鍵完全沒反應
 *   八成是排線的列與行接反了。把 rowPins 和 colPins 兩行對調試試。
 *
 * 按 1 卻顯示 4
 *   排線接的順序不對，或鍵盤的列行定義和 keys 陣列不一致。
 *   先寫一支只印 keypad.getKey() 的小程式，把每一顆實際按一次記下來，
 *   再回頭改 keys 陣列。
 *
 * 這支程式把密碼寫死在程式裡，而且斷電後輸入就清空，
 * 純粹是教學用的。真正的門禁不會這樣做。
 * 想讓密碼可以修改又能保留，要寫進 EEPROM。
 */
