# VS Code 工作用的 Arduino 輔助腳本
# 由 tasks.json 呼叫，學員不需要直接執行。
#
#   -Action compile   只編譯
#   -Action upload    編譯並上傳（自動找 COM 埠）
#   -Action monitor   開序列埠監控（鮑率自動讀程式裡的 Serial.begin）
#   -Action ports     列出所有連接埠
#
# 自動偵測猜錯時，可以設環境變數 ARDUINO_PORT 指定，例如 $env:ARDUINO_PORT = "COM5"

param(
  [Parameter(Mandatory)][ValidateSet('compile', 'upload', 'monitor', 'ports')][string]$Action,
  [string]$File = ''
)

$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.Encoding]::UTF8
$FQBN = 'arduino:avr:uno'
$LastSketch = Join-Path $env:TEMP 'arduino-last-sketch.txt'   # 上傳成功時記下，給序列埠監控讀鮑率

function Fail($msg) {
  Write-Host ''
  Write-Host "✗ $msg" -ForegroundColor Red
  exit 1
}

# ── 找 arduino-cli：先找 PATH，再找 Arduino IDE 內建的那一份 ──
$cli = (Get-Command arduino-cli -ErrorAction SilentlyContinue).Source
if (-not $cli) {
  $bundled = Join-Path $env:LOCALAPPDATA 'Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
  if (Test-Path $bundled) { $cli = $bundled }
}
if (-not $cli) { Fail '找不到 arduino-cli。請先安裝 Arduino IDE 2.x，或執行 winget install ArduinoSA.CLI' }

# ── 找程式資料夾：用目前編輯中的檔案所在的資料夾 ──
function Get-SketchDir {
  if (-not $File) { Fail '請先在編輯器裡打開一個 .ino 檔，再執行這個工作。' }
  $dir = Split-Path $File -Parent
  $name = Split-Path $dir -Leaf
  if (-not (Test-Path (Join-Path $dir "$name.ino"))) {
    Fail "資料夾「$name」裡找不到 $name.ino。`n  Arduino 規定資料夾名稱要和 .ino 檔名相同，請先打開某一個單元的 .ino 檔。"
  }
  return $dir
}

# ── 自動找 Arduino 接在哪個 COM 埠 ──
function Get-Port {
  if ($env:ARDUINO_PORT) { return $env:ARDUINO_PORT }

  $ports = (& $cli board list --format json | ConvertFrom-Json).detected_ports |
    Where-Object { $_.port.protocol -eq 'serial' }

  # 1. 官方板：arduino-cli 認得出是 Uno
  $uno = @($ports | Where-Object { $_.matching_boards.fqbn -contains $FQBN })
  if ($uno.Count -eq 1) { return $uno[0].port.address }

  # 2. 相容板（CH340 等）：認不出型號，但一定是 USB 序列埠（有 vid）
  #    COM1、COM2 這類主機板內建的埠沒有 vid，會被排除
  $usb = @($ports | Where-Object { $_.port.properties.vid })
  if ($usb.Count -eq 1) { return $usb[0].port.address }

  if ($usb.Count -eq 0) {
    Fail "找不到 Arduino。請檢查：`n  1. USB 線有沒有插好（有些線只能充電、不能傳資料）`n  2. 相容板要先裝 CH340 驅動`n  3. 執行「Arduino: 列出連接埠」看看有沒有出現新的 COM"
  }
  $list = ($usb | ForEach-Object { $_.port.address }) -join '、'
  Fail "偵測到不只一個 USB 序列埠：$list`n  請拔掉其他裝置，或在終端機先執行 `$env:ARDUINO_PORT = `"COM5`" 指定。"
}

# ── 上傳前關掉序列埠監控，否則 COM 埠被占用會上傳失敗 ──
function Stop-Monitor {
  $mon = Get-CimInstance Win32_Process -Filter "Name = 'arduino-cli.exe'" |
    Where-Object { $_.CommandLine -match '\smonitor\s' }
  if ($mon) {
    Write-Host '先關閉正在執行的序列埠監控…' -ForegroundColor DarkGray
    $mon | ForEach-Object { Stop-Process -Id $_.ProcessId -Force }
    Start-Sleep -Milliseconds 500
  }
}

switch ($Action) {
  'ports' {
    & $cli board list
  }
  'compile' {
    $dir = Get-SketchDir
    Write-Host "編譯 $(Split-Path $dir -Leaf) …" -ForegroundColor Cyan
    & $cli compile --fqbn $FQBN $dir
    if ($LASTEXITCODE -ne 0) { Fail '編譯失敗，請看上方的錯誤訊息（也會標在「問題」面板）。' }
    Write-Host '✓ 編譯成功' -ForegroundColor Green
  }
  'upload' {
    $dir = Get-SketchDir
    $port = Get-Port
    Stop-Monitor
    Write-Host "編譯並上傳 $(Split-Path $dir -Leaf) → $port …" -ForegroundColor Cyan
    & $cli compile --fqbn $FQBN -u -p $port $dir
    if ($LASTEXITCODE -ne 0) { Fail "上傳失敗。如果是 Access is denied，代表 $port 被別的程式占用（例如 Arduino IDE 的序列埠監控）。" }
    Write-Host '✓ 上傳完成' -ForegroundColor Green
    Set-Content $LastSketch (Join-Path $dir "$(Split-Path $dir -Leaf).ino") -Encoding UTF8
  }
  'monitor' {
    $port = Get-Port
    $baud = 9600
    # 監控工作不帶 ${file}：上傳完焦點停在終端機時，VS Code 會拒絕執行帶 ${file} 的工作。
    # 改讀最後一次上傳的程式，板子上跑的本來就是它。
    if (-not $File -and (Test-Path $LastSketch)) { $File = (Get-Content $LastSketch -TotalCount 1).Trim() }
    if ($File -and (Test-Path $File)) {
      Write-Host "鮑率依據：$(Split-Path $File -Leaf)" -ForegroundColor DarkGray
      $m = Select-String -Path $File -Pattern 'Serial\.begin\(\s*(\d+)' | Select-Object -First 1
      if ($m) { $baud = [int]$m.Matches[0].Groups[1].Value }
    }
    Write-Host "序列埠監控 $port @ $baud（按 Ctrl+C 結束）" -ForegroundColor Cyan
    & $cli monitor -p $port -c "baudrate=$baud"
  }
}
