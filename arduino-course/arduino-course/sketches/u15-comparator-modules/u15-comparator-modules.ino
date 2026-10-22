/*
 * U15 · 火焰、水位、循跡：一次認識三個同類模組
 *
 * 這三個模組長得不一樣，用法卻是同一套：
 *   AO（類比輸出）給你連續的數值，DO（數位輸出）只給你「有／沒有」。
 *   模組上的可變電阻是在調 DO 的門檻，不影響 AO。
 *
 * 建議一律接 AO，自己在程式裡定門檻 —— 這樣門檻改起來不必動螺絲起子。
 *
 * 接線：
 *   火焰感測 AO ── A0
 *   水位感測 S  ── A1
 *   循跡模組 AO ── A2      （三個模組的 VCC 都接 5V、GND 都接 GND）
 */

const int PIN_FLAME = A0;
const int PIN_WATER = A1;
const int PIN_TRACK = A2;

// 這三個數字全部都要現場量過再改
const int FLAME_THRESHOLD = 400;   // 低於此值 = 偵測到火（越近讀值越低）
const int WATER_THRESHOLD = 300;   // 高於此值 = 碰到水（越深讀值越高）
const int TRACK_THRESHOLD = 500;   // 低於此值 = 黑線（黑色反射弱）

void setup() {
  Serial.begin(9600);
  Serial.println("原始值\t\t判讀");
  Serial.println("火焰\t水位\t循跡");
}

void loop() {
  int flame = analogRead(PIN_FLAME);
  int water = analogRead(PIN_WATER);
  int track = analogRead(PIN_TRACK);

  Serial.print(flame); Serial.print("\t");
  Serial.print(water); Serial.print("\t");
  Serial.print(track); Serial.print("\t→ ");

  if (flame < FLAME_THRESHOLD) Serial.print("有火 ");
  if (water > WATER_THRESHOLD) Serial.print("有水 ");
  if (track < TRACK_THRESHOLD) Serial.print("黑線 ");
  Serial.println();

  delay(300);
}

/*
 * 三個模組的方向不一樣，注意大於還是小於
 *   火焰：越靠近火，讀值越低   → 用 <
 *   水位：泡得越深，讀值越高   → 用 >
 *   循跡：黑色反射弱、讀值低   → 用 <
 *
 * 定門檻的標準流程
 *   1. 上傳後開序列埠監控視窗
 *   2. 記下「平常」的讀值
 *   3. 記下「觸發」的讀值（打火機靠近／沾水／放到黑線上）
 *   4. 取中間值
 *
 * 水位感測器不要整支泡下去，只泡有金屬梳齒的那一段；
 * 長期泡在水裡會電解腐蝕，測完就拿起來。
 */
