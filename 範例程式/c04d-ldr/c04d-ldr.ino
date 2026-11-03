/*
 * 第四章 · 感測器 4：LM393 光敏模組
 *
 * 目標：認識類比輸入（analogRead），以及「門檻要現場量」。
 * 接線（模組有四支針腳，印刷順序 AO、DO、GND、VCC）：
 *   VCC ── 紅軌（5V）   GND ── 藍軌（GND）   AO ── Uno A1
 *   DO 不接（它只有「亮／暗」兩種狀態，本課程用 AO 讀數值）
 *
 * 原理：光敏電阻遇到光，電阻變小。模組把電阻變化轉成電壓，
 *       Uno 用 analogRead 把電壓換成 0 到 1023 的數字。
 *
 * 注意：這個模組是「越暗，數字越大」，和直覺相反！
 */

const int LDR_PIN = A1;
const int DARK_THRESHOLD = 400;   // 超過這個數字算「暗」。這只是範例值，一定要現場量！

bool lastDark = false;

void setup() {
  Serial.begin(9600);
  Serial.println("LM393 光敏模組開始讀取");
}

void loop() {
  int value = analogRead(LDR_PIN);          // 0（最亮）到 1023（最暗）
  bool dark = value > DARK_THRESHOLD;

  Serial.print("光線數值 ");
  Serial.print(value);
  Serial.println(dark ? "  （暗）" : "  （亮）");

  // 狀態改變的那一瞬間，多印一行
  if (dark != lastDark) {
    lastDark = dark;
    Serial.println(dark ? ">>> 變暗了" : ">>> 變亮了");
  }

  delay(300);
}

/*
 * 動手試試
 *   1. 先讓感測器正常照著室內光，記下平常的數字。
 *   2. 用手把感測器完全遮住，記下遮住的數字。
 *   3. 門檻取兩個數字的中間值。例如平常 280、遮住 490，門檻大約填 400。
 *      每個模組、每個教室的光都不一樣，所以不能照抄範例的 400。
 *   4. 模組上的藍色旋鈕是調 DO 的門檻，不影響 AO 的數字，這支程式用不到。
 *
 * 常見狀況
 *   數字一直是 0 或 1023 → AO 沒接在 A1，或 VCC／GND 沒接好。
 *   遮住和不遮住差不多 → 接到 DO 而不是 AO。
 */
