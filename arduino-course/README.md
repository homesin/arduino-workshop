# Arduino 完整課程

從第一次點亮 LED，到一台會把資料送上雲端、異常時傳 LINE 的裝置。
用的是 **UNO R3 學習套件(B)** 裡的零件，沒有額外採購。

**每一支程式都用 arduino-cli 對 Arduino Uno 實際編譯驗證過**（22/22 通過）。

---

## 課程地圖

### 第一篇 · 讓板子聽話（數位與類比輸出）

| 單元 | 主題 | 學到什麼 | 零件 |
|---|---|---|---|
| U01 | [讓板子上的燈閃起來](sketches/u01-blink) | `setup()` / `loop()` 的分工 | 只要板子 |
| U02 | [接一顆自己的 LED](sketches/u02-external-led) | 極性、限流電阻、為什麼一定要串 | LED、220Ω |
| U03 | [五顆燈的跑馬燈](sketches/u03-led-chase) | 陣列與 `for`，取代五份複製貼上 | LED ×5、220Ω ×5 |
| U04 | [呼吸燈](sketches/u04-breathing-led) | PWM：`analogWrite` 與 `~` 記號的腳位 | LED、220Ω |
| U05 | [RGB 混色](sketches/u05-rgb-mix) | 三路 PWM、共陰與共陽 | RGB LED、220Ω ×3 |

### 第二篇 · 讓板子讀懂外界（輸入）

| 單元 | 主題 | 學到什麼 | 零件 |
|---|---|---|---|
| U06 | [按鈕](sketches/u06-button) | `INPUT_PULLUP`、接點彈跳與去彈跳 | 輕觸開關 |
| U07 | [可變電阻](sketches/u07-potentiometer) | `analogRead` 的 0–1023、`map()` 換算 | 10K 可變電阻 |
| U08 | [光敏電阻自動夜燈](sketches/u08-photoresistor) | 分壓電路、門檻要現場量 | 光敏電阻、10KΩ |
| U09 | [搖桿](sketches/u09-joystick) | 多路類比、靜止區 | JoyStick 模組 |

### 第三篇 · 時間與聲音

| 單元 | 主題 | 學到什麼 | 零件 |
|---|---|---|---|
| U10 | [蜂鳴器](sketches/u10-buzzer) | `tone()` 控制頻率、音階、Timer 衝突 | 無源蜂鳴器 |
| U11 | **[不用 delay 也能同時做兩件事](sketches/u11-millis-multitask)** | `millis()` 非阻塞寫法 | LED ×2、按鈕 |

> **U11 是整套課程最重要的一課。** 後面所有「要同時做很多事」的程式都建立在它上面。
> 沒學會這個，整合專題一定寫不出來。

### 第四篇 · 感測器

| 單元 | 主題 | 學到什麼 | 零件 |
|---|---|---|---|
| U12 | [DHT11 溫溼度](sketches/u12-dht11) | 第一次用外部函式庫、讀取失敗要怎麼處理 | DHT11 模組 |
| U13 | [超音波測距](sketches/u13-ultrasonic) | `pulseIn` 量時間差、自己換算距離 | HC-SR04 |
| U14 | [人體紅外線](sketches/u14-pir) | 邊緣觸發：只在狀態改變時動作 | HC-SR501 |
| U15 | [火焰 / 水位 / 循跡](sketches/u15-comparator-modules) | AO 與 DO 的差別、三個模組同一套用法 | 三個比較器模組 |

### 第五篇 · 顯示與動作

| 單元 | 主題 | 學到什麼 | 零件 |
|---|---|---|---|
| U16 | [七段顯示器 + 74HC595](sketches/u16-seven-segment) | 移位暫存器：3 支腳控制 8 個輸出 | 七段、74HC595 |
| U17 | [伺服馬達](sketches/u17-servo) | 角度控制、供電不足會讓板子重開 | SG90 |
| U18 | [步進馬達](sketches/u18-stepper) | 走幾步 vs 轉到幾度 | 28BYJ-48 + ULN2003 |
| U19 | [直流馬達](sketches/u19-dc-motor) | 為什麼不能直接接腳位、正反轉與調速 | L9110 |
| U20 | [矩陣鍵盤密碼鎖](sketches/u20-keypad-lock) | 矩陣掃描、把前面學的組起來 | 4x4 薄膜鍵盤 |

### 第六篇 · 接上電腦

| 單元 | 主題 | 學到什麼 |
|---|---|---|
| U21 | [設計序列埠協定](sketches/u21-serial-protocol) | 為什麼用 JSON、`#` 前綴、雙向溝通 |
| U22 | Node.js 讀取序列埠 | `serialport` 套件 → 見 [`../arduino-cloud-workshop`](../arduino-cloud-workshop) |
| U23 | 即時儀表板 | Express + Socket.IO |
| U24 | 從瀏覽器控制 Arduino | 指令白名單、寫回序列埠 |

### 第七篇 · 上雲端

| 單元 | 主題 | 學到什麼 |
|---|---|---|
| U25 | ThingSpeak | 長期存檔、公開圖表、15 秒節流 |
| U26 | LINE 推播 | broadcast 不需要 webhook、額度怎麼算 |
| U27 | LINE 問答機器人 | reply 免費無限，代價是要架 webhook |

第六、七篇的程式都在 [`../arduino-cloud-workshop`](../arduino-cloud-workshop)，該資料夾有自己的 README。

### 整合專題

**[校園環境監測站](sketches/p1-monitor-station)** —— 把整套東西組成一台放著就能跑的裝置。

量測（溫溼度、距離、火焰、光線、水位）→ 顯示（七段狀態碼、雙色 LED）→
反應（蜂鳴器警報、伺服馬達舉旗）→ 回報（每秒一行 JSON）→ 受控（電腦可調門檻、消音、測試）。

整支程式**沒有一個 `delay()`** —— 因為它要同時做四件節奏不同的事。

---

## 整合專題的腳位表

| 腳位 | 接什麼 | 備註 |
|---|---|---|
| D2 | DHT11 DATA | |
| D3 | HC-SR501 PIR OUT | |
| D4 | HC-SR04 Trig | |
| D5 | HC-SR04 Echo | |
| D6 | 無源蜂鳴器 | `tone()` 占用 Timer2 |
| D7 | 紅色 LED（警戒）| 串 220Ω |
| D8 | 綠色 LED（正常）| 串 220Ω |
| D9 | SG90 訊號 | Servo 占用 Timer1 |
| D10 | 74HC595 DS (pin 14) | |
| D11 | 74HC595 STCP (pin 12) | |
| D12 | 74HC595 SHCP (pin 11) | |
| A0 | 火焰感測 AO | |
| A1 | 光敏電阻分壓中點 | |
| A2 | 水位感測 S | |
| A3 | 可變電阻中間腳 | 現場即時調火焰門檻 |

D0 / D1 保留給序列埠，不要用。

### 狀態碼（顯示在七段上）

| 碼 | 意義 |
|---|---|
| 0 | 正常 |
| 1 | 偵測到有人 |
| 2 | 有東西靠近（15 公分內）|
| 3 | 淹水 |
| 4 | 火警 |

---

## 開始之前

### 需要安裝的函式庫

Arduino IDE → **工具 → 管理程式庫**，搜尋安裝：

| 函式庫 | 作者 | 用在 |
|---|---|---|
| SimpleDHT | Winlin | U12、U21、整合專題 |
| Keypad | Mark Stanley | U20 |

**Servo** 與 **Stepper**（U17、U18、整合專題用到）是 Arduino IDE 內建的，不用另外安裝。

### 進化版 Uno 的驅動

多數進化版用的是 CH340 晶片。若「工具 → 連接埠」找不到 COM，就是驅動沒裝。

---

## 三個貫穿全課的觀念

**1. 門檻一定要現場量，不要照抄。**
每一顆類比感測器、每個場地的環境都不一樣。標準流程是：先上傳、開序列埠監控視窗、
記下「平常」和「觸發」兩個讀值、取中間值。U08、U15 和整合專題都在做同一件事。

**2. 只在狀態改變的那一瞬間動作。**
PIR 觸發後會維持 HIGH 好幾秒。每次迴圈都動作的話，序列埠會被洗版、LINE 額度幾秒就用光。
U14 教這個寫法，U26 的 LINE 通報直接沿用。

**3. 需要同時做兩件事就不能用 `delay()`。**
U11 是分水嶺。在那之前的程式都是一條直線，之後的都是「檢查時間到了沒」。

---

## 自己驗證所有程式

不用接板子，也能確認 22 支程式都編得過：

```bash
# Arduino IDE 2.x 內建 arduino-cli，路徑在
# %LOCALAPPDATA%\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe

arduino-cli lib install SimpleDHT Keypad Servo Stepper

for /d %d in (sketches\*) do arduino-cli compile --fqbn arduino:avr:uno "%d"
```

PowerShell 版本：

```powershell
$cli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
Get-ChildItem sketches -Directory | ForEach-Object {
  & $cli compile --fqbn arduino:avr:uno $_.FullName
  Write-Output "$($_.Name) -> exit $LASTEXITCODE"
}
```

---

## 用 VS Code 代替 Arduino IDE（選用）

`.vscode/` 裡有設定好的工作。用 VS Code 打開 `arduino-course` 這個資料夾，
或打開上一層的整個專案資料夾（那裡也放了一份指向同一支腳本的設定），再打開任一單元的 `.ino` 檔：

| 操作 | 作用 |
|---|---|
| `Ctrl+Shift+B` | 編譯並上傳到板子，會自動找 COM 埠 |
| 執行工作 →「Arduino: 只編譯」 | 不接板子也能檢查語法 |
| 執行工作 →「Arduino: 序列埠監控」 | 鮑率自動讀最後上傳那支程式的 `Serial.begin()` |
| 執行工作 →「Arduino: 列出連接埠」 | 找不到板子時用來排查 |

「執行工作」在上方選單「**終端機 → 執行工作…**」（英文介面是 Terminal → Run Task…）；
也可以按 `Ctrl+Shift+P` 輸入 `run task`，中英文介面都認得這個英文關鍵字。

- 第一次打開資料夾時，VS Code 會問「是否信任此資料夾中檔案的作者」，**一定要選「是」**，選「否」會進入限制模式，上面的工作全部不能用。
- 出現「無法解析變數 ${file}。請開啟編輯器。」表示當下沒有作用中的程式檔（例如停在歡迎頁、設定頁或預覽頁）。先點一下要上傳的 `.ino` 分頁再執行即可。「序列埠監控」與「列出連接埠」不需要開檔，監控的鮑率會讀最後一次上傳的那支程式。
- `.ino` 已設定成用 C++ 上色（`.vscode/settings.json`），不用另外裝擴充套件。
- 編譯錯誤會標在「問題」面板，點一下就跳到出錯的那一行。
- 上傳前會自動關掉序列埠監控，不必手動關。
- 同時插著兩塊板子時自動偵測會拒絕猜測，要先在終端機設定 `$env:ARDUINO_PORT = "COM5"`。
- 需要先裝好 arduino-cli（裝 Arduino IDE 2.x 就會附一份）和上面列出的函式庫。
- VS Code 沒有 IDE 的**序列埠繪圖器**，要看波形的單元（U07、U08）還是用 Arduino IDE 比較直觀。

---

## 教學時間怎麼安排

課程本身不綁時數，依對象調整：

| 對象 | 建議範圍 | 大約時數 |
|---|---|---|
| 一次性體驗 | U01 U02 U04 | 1 小時 |
| 同仁研習（單場）| U01–U04 + U12 + 第六七篇 | 3 小時 |
| 完整入門 | U01–U15 | 8–10 小時 |
| 全課 + 專題 | U01–U27 + 整合專題 | 20–24 小時 |

無論選哪一段，**U11 都建議保留**。
