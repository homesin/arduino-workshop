# 學員電腦一鍵安裝＋驗收
#
# 用法：雙擊同資料夾的 install.cmd（會自動用正確的參數呼叫這支腳本）
#
#   install.cmd                 全部裝：Arduino IDE、板子核心、函式庫、CH340 驅動、VS Code、Node.js、專案套件、Codex CLI
#   install.cmd -ArduinoOnly    只裝 Arduino 部分（含 CH340 驅動與 VS Code），不裝 Node.js 與 Codex CLI（上午場只教硬體時用）
#
# 軟體都用 winget 安裝；電腦上沒有 winget 的話會先自動補裝（見 Install-Winget）。
# 已經裝過的項目會自動跳過，重複執行沒有副作用。

param([switch]$ArduinoOnly)

# 用 Continue 而不是 Stop：Windows PowerShell 5.1 在導向外部程式的 stderr 時，
# 會把每一行警告都當成錯誤直接中止。錯誤一律自己檢查 $LASTEXITCODE。
$ErrorActionPreference = 'Continue'
[Console]::OutputEncoding = [Text.Encoding]::UTF8
# 關掉 PowerShell 自己的進度區：5.1 會把它畫在視窗上方、蓋掉說明文字，
# Add-AppxPackage 失敗時還會只留下一個嚇人的「Error」；Invoke-WebRequest 也會因此慢十倍。
# winget、npm 的下載進度條是它們自己畫的，不受影響。
$ProgressPreference = 'SilentlyContinue'

$Root      = Split-Path $PSScriptRoot -Parent
$Course    = Join-Path $Root 'arduino-course\sketches'
$Workshop  = Join-Path $Root 'arduino-cloud-workshop'
$FQBN      = 'arduino:avr:uno'
$Libraries = 'SimpleDHT', 'Keypad', 'Servo', 'Stepper'
$CH340Url  = 'https://www.wch-ic.com/download/file?id=65'   # WCH 官網的 CH341SER.EXE
$WingetFiles   = 'DesktopAppInstaller_Dependencies.zip', 'Microsoft.DesktopAppInstaller_8wekyb3d8bbwe.msixbundle'
$WingetOffline = Join-Path $PSScriptRoot 'winget-offline'   # 講師預先放的離線安裝檔（見該資料夾 README）
$Results   = [System.Collections.Generic.List[object]]::new()

function Step($msg)  { Write-Host ''; Write-Host "▶ $msg" -ForegroundColor Cyan }
function Info($msg)  { Write-Host "  $msg" -ForegroundColor DarkGray }
function Record($item, $ok, $note = '') {
  $Results.Add([pscustomobject]@{ 項目 = $item; 結果 = $(if ($ok -eq $true) { '✓' } elseif ($ok -eq $false) { '✗' } else { '！' }); 說明 = $note })
}
function Abort($msg) {
  Write-Host ''; Write-Host "✗ $msg" -ForegroundColor Red
  exit 1
}

# winget 裝完的程式要重新讀 PATH 才找得到
function Update-Path {
  $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' +
              [Environment]::GetEnvironmentVariable('Path', 'User')
}

function Install-WithWinget($id, $name, [string[]]$extra = @()) {
  Info "用 winget 安裝 $name（第一次會跳出同意條款或系統管理員確認，請按「是」）…"
  winget install --id $id -e --silent --accept-package-agreements --accept-source-agreements @extra
  Update-Path
}

# winget 是 Windows 10 1809 以後內建的「應用程式安裝程式」，但下面幾種學員電腦會找不到：
#   - 新帳號第一次登入，Store 還沒在背景註冊完（還原卡、共用帳號的電腦教室常見）
#   - 資訊組封鎖了 Microsoft Store
#   - Windows 10 LTSC、很久沒更新的 Windows 10、Windows Sandbox
# 全部用 Add-AppxPackage 裝在目前使用者底下，不需要系統管理員權限。
function Install-Winget {
  Info '這台電腦找不到 winget，嘗試自動補上…'

  # 1. 套件其實在，只是還沒註冊到這個帳號：幾秒就好
  try {
    Add-AppxPackage -RegisterByFamilyName -MainPackage 'Microsoft.DesktopAppInstaller_8wekyb3d8bbwe' -ErrorAction Stop
    Update-Path
    if (Get-Command winget -ErrorAction SilentlyContinue) { Info '重新註冊成功'; return }
  } catch { }

  # 2. 從離線資料夾或 GitHub 安裝。講師可以先把兩個檔案放進 setup\winget-offline\，
  #    用隨身碟發給學員就不必每台都下載 300 MB（Sandbox 測試也是走這條）。
  [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
  $base    = 'https://github.com/microsoft/winget-cli/releases/latest/download'
  $tmp     = Join-Path $env:TEMP 'winget-setup'
  New-Item -ItemType Directory -Force $tmp | Out-Null
  try {
    foreach ($f in $WingetFiles) {
      $local = Join-Path $WingetOffline $f
      if (Test-Path $local) {
        Info "使用教材附的 $f"
        Copy-Item $local $tmp -Force
        continue
      }
      # GitHub 在學校網路有時只有幾十 KB/s，設長一點的逾時並重試一次
      foreach ($try in 1, 2) {
        Info "從 GitHub 下載 $f（第 $try 次，約 100–200 MB，網路慢的話要很久）…"
        try {
          Invoke-WebRequest "$base/$f" -OutFile (Join-Path $tmp $f) -UseBasicParsing -TimeoutSec 900 -ErrorAction Stop
          break
        } catch {
          Info "  失敗：$($_.Exception.Message)"
          if ($try -eq 2) { throw "無法下載 $f" }
        }
      }
    }
    Info '安裝 winget 與相依套件…'
    Expand-Archive (Join-Path $tmp $WingetFiles[0]) (Join-Path $tmp 'deps') -Force
    $arch = if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'arm64' } else { 'x64' }
    $deps = Get-ChildItem (Join-Path $tmp 'deps') -Recurse -Include *.appx, *.msix |
      Where-Object { $_.FullName -match "\\$arch\\" } | ForEach-Object FullName
    Add-AppxPackage -Path (Join-Path $tmp $WingetFiles[1]) -DependencyPath $deps -ErrorAction Stop
    Update-Path
  } catch {
    Info "  失敗：$($_.Exception.Message)"
  } finally {
    Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
  }
}

function Test-CH340Driver {
  [bool](Get-ChildItem (Join-Path $env:WINDIR 'System32\DriverStore\FileRepository') -Filter 'ch341ser.inf_*' -Directory -ErrorAction SilentlyContinue)
}

function Find-VSCode {
  $cmd = Get-Command code -ErrorAction SilentlyContinue
  if ($cmd) { return $cmd.Source }
  foreach ($dir in (Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code'), (Join-Path $env:ProgramFiles 'Microsoft VS Code')) {
    $bin = Join-Path $dir 'bin\code.cmd'
    if (Test-Path $bin) { return $bin }
  }
  return $null
}

function Find-ArduinoCli {
  $cmd = Get-Command arduino-cli -ErrorAction SilentlyContinue
  if ($cmd) { return $cmd.Source }
  $bundled = Join-Path $env:LOCALAPPDATA 'Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
  if (Test-Path $bundled) { return $bundled }
  return $null
}

# ─────────────────────────────────────────────
Step '0. 檢查環境'
if (-not (Test-Path $Course)) { Abort "找不到 $Course，請確認這支腳本放在教材資料夾的 setup\ 底下。" }
Info "教材位置：$Root"
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) { Install-Winget }
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
  Abort "winget 自動安裝失敗，後面的軟體都靠它安裝，請找講師。`n  （講師：可到 Microsoft Store 更新「應用程式安裝程式」，或把 winget 安裝檔放進 setup\winget-offline\ 後重跑）"
}
Record 'winget' $true (winget --version)

# ─────────────────────────────────────────────
Step '1. Arduino IDE（內含 arduino-cli）'
$cli = Find-ArduinoCli
if ($cli) {
  Info '已安裝，跳過'
} else {
  # 指定 exe 安裝檔（和官網下載的同一個，裝在使用者目錄）。
  # 不指定的話，系統管理員身分下 winget 會挑 MSI，它回報成功卻沒有放任何檔案。
  Install-WithWinget 'ArduinoSA.IDE.stable' 'Arduino IDE' '--installer-type', 'nullsoft'
  $cli = Find-ArduinoCli
  if (-not $cli) { Abort 'Arduino IDE 安裝失敗。請手動到 arduino.cc/en/software 下載安裝後再執行一次。' }
}
Record 'Arduino IDE / arduino-cli' $true (& $cli version)

# ─────────────────────────────────────────────
Step '2. Uno 板子核心（arduino:avr）'
& $cli core update-index | Out-Null
$hasCore = (& $cli core list --format json | ConvertFrom-Json).platforms.id -contains 'arduino:avr'
if ($hasCore) {
  Info '已安裝，跳過'
} else {
  Info '下載中，大約 100 MB，網路慢的話要幾分鐘…'
  & $cli core install arduino:avr
}
$hasCore = (& $cli core list --format json | ConvertFrom-Json).platforms.id -contains 'arduino:avr'
Record '板子核心 arduino:avr' $hasCore

# ─────────────────────────────────────────────
Step "3. 函式庫：$($Libraries -join '、')"
# Servo、Stepper 雖然 IDE 內建，但 arduino-cli（VS Code 那條路）看不到內建的那份，一樣要裝
& $cli lib install @Libraries
Record '函式庫' ($LASTEXITCODE -eq 0) ($Libraries -join ' ')

# ─────────────────────────────────────────────
Step '4. CH340 驅動（相容板的 USB 晶片）'
# 原廠 Uno 不需要；相容板雖然 Windows Update 通常會自動抓，但學校電腦常關掉自動安裝驅動或沒網路。
# 不跑 CH341SER.EXE 的安裝精靈（會跳視窗要按按鈕），改成解開後用 pnputil 直接裝進驅動程式庫，
# 這樣沒插板子也能先裝好，之後插上任何一塊都直接認得。
if (Test-CH340Driver) {
  Info '已安裝，跳過'
  Record 'CH340 驅動' $true
} else {
  $tmp = Join-Path $env:TEMP 'ch341ser'
  $exe = Join-Path $env:TEMP 'CH341SER.EXE'
  $note = ''
  try {
    Info '從 WCH 官網下載 CH341SER.EXE…'
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest $CH340Url -OutFile $exe -UseBasicParsing -ErrorAction Stop

    $sig = Get-AuthenticodeSignature $exe
    if ($sig.Status -ne 'Valid' -or $sig.SignerCertificate.Subject -notmatch 'Qinheng') {
      throw '下載的檔案簽章不是 WCH（南京沁恒），不安裝'
    }

    # 它是 RAR 自解壓檔，Windows 10 1803 起內建的 tar 就解得開
    Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory $tmp | Out-Null
    & "$env:WINDIR\System32\tar.exe" -xf $exe -C $tmp
    $inf = Join-Path $tmp 'WIN 1X\CH341SER.INF'
    if (-not (Test-Path $inf)) { throw '解壓縮失敗，找不到 CH341SER.INF' }

    Info '安裝驅動（會跳出系統管理員確認，請按「是」）…'
    $p = Start-Process pnputil -ArgumentList '/add-driver', "`"$inf`"", '/install' -Verb RunAs -Wait -PassThru -WindowStyle Hidden
    if ($p.ExitCode -eq 3010) { $note = '已安裝，重新開機後生效' }
  } catch {
    $note = $_.Exception.Message
  } finally {
    Remove-Item $tmp, $exe -Recurse -Force -ErrorAction SilentlyContinue
  }
  if (Test-CH340Driver) {
    Record 'CH340 驅動' $true $note
  } else {
    # 用「！」不用「✗」：原廠 Uno 不需要它，Windows Update 也可能插板子時自動補上
    Record 'CH340 驅動' $null "沒裝成（$note）。用相容板的話，手動到 WCH 官網下載 CH341SER.EXE 安裝"
  }
}

# ─────────────────────────────────────────────
Step '5. VS Code（選用：arduino-course 附了一鍵編譯上傳的工作）'
$code = Find-VSCode
if ($code) {
  Info '已安裝，跳過'
} else {
  # 指定 user scope：和 Arduino IDE 同樣的理由，系統管理員身分下 winget 可能改挑全機版安裝檔。
  # 靜默安裝只套用預設勾選，桌面圖示與右鍵「以 Code 開啟」預設沒勾，要自己加進 MERGETASKS。
  Install-WithWinget 'Microsoft.VisualStudioCode' 'VS Code' '--scope', 'user', '--custom',
    '/MERGETASKS=!runcode,desktopicon,addcontextmenufiles,addcontextmenufolders,associatewithfiles,addtopath'
  $code = Find-VSCode
  # 保險：萬一安裝程式沒吃到上面的參數，自己建桌面捷徑
  $lnk = Join-Path ([Environment]::GetFolderPath('Desktop')) 'Visual Studio Code.lnk'
  if ($code -and -not (Test-Path $lnk)) {
    $exe = Join-Path (Split-Path (Split-Path $code -Parent) -Parent) 'Code.exe'   # bin\code.cmd → Code.exe
    if (Test-Path $exe) {
      $sc = (New-Object -ComObject WScript.Shell).CreateShortcut($lnk)
      $sc.TargetPath = $exe
      $sc.Save()
      Info '已在桌面建立 VS Code 捷徑'
    }
  }
}
if ($code) {
  Record 'VS Code' $true (@(& $code --version)[0])
  # VS Code 預設只有英文介面，講義寫的「執行工作」在英文版叫 Tasks: Run Task，學員會找不到。
  # 裝了語言套件後，VS Code 會跟著 Windows 的顯示語言自動切成中文。
  $zh = 'ms-ceintl.vscode-language-pack-zh-hant'
  if (@(& $code --list-extensions) -contains $zh) {
    Info '繁體中文語言套件已安裝，跳過'
  } else {
    Info '安裝 VS Code 繁體中文語言套件…'
    & $code --install-extension $zh *> $null
  }
  if (@(& $code --list-extensions) -contains $zh) {
    Record 'VS Code 中文介面' $true
  } else {
    Record 'VS Code 中文介面' $null '沒裝成，介面會是英文：「執行工作」在英文版叫 Tasks: Run Task'
  }
} else {
  # 用「！」：Arduino IDE 已經能完成全部單元，VS Code 是選用
  Record 'VS Code' $null '沒裝成。要用的話到 code.visualstudio.com 手動下載；不裝也能用 Arduino IDE 上課'
}

# ─────────────────────────────────────────────
if (-not $ArduinoOnly) {
  Step '6. Node.js（20 版以上）'
  Update-Path
  $nodeMajor = 0
  if (Get-Command node -ErrorAction SilentlyContinue) { $nodeMajor = [int]((node -v) -replace '^v(\d+).*', '$1') }
  if ($nodeMajor -ge 20) {
    Info "已安裝 $(node -v)，跳過"
  } else {
    if ($nodeMajor -gt 0) { Info "目前是 $(node -v)，版本太舊，改裝 LTS 版" }
    Install-WithWinget 'OpenJS.NodeJS.LTS' 'Node.js LTS'
    if (-not (Get-Command node -ErrorAction SilentlyContinue)) {
      Abort 'Node.js 安裝後仍找不到 node。請關掉這個視窗重新執行一次（PATH 有時要重開才生效）。'
    }
  }
  Record 'Node.js' ($true) (node -v)

  Step '7. 雲端專案的套件（npm install）'
  if (-not (Test-Path $Workshop)) {
    Record '專案套件' $null "找不到 $Workshop，跳過"
  } else {
    Push-Location $Workshop
    try {
      if (Test-Path 'node_modules') {
        Info '已安裝，跳過'
      } else {
        npm install --no-fund --no-audit
      }
      Record '專案套件' (Test-Path 'node_modules\serialport')
      if (-not (Test-Path '.env') -and (Test-Path '.env.example')) {
        Copy-Item '.env.example' '.env'
        Record '.env 設定檔' $null '已從 .env.example 複製，金鑰要自己填'
      }
    } finally { Pop-Location }
  }

  Step '8. Codex CLI（上課卡關時問 AI 用）'
  # npm 全域安裝會產生 codex.ps1，Windows 預設的 Restricted 政策會擋掉它，
  # 學員在 PowerShell 或 VS Code 終端機打 codex 會直接報錯。npm.ps1 也是同樣問題。
  if ((Get-ExecutionPolicy) -in 'Restricted', 'AllSigned', 'Undefined') {
    Set-ExecutionPolicy -Scope CurrentUser RemoteSigned -Force -ErrorAction SilentlyContinue
    Info "PowerShell 執行政策改為 RemoteSigned（僅限目前使用者），codex 與 npm 指令才能執行"
  }
  if (Get-Command codex -ErrorAction SilentlyContinue) {
    Info '已安裝，跳過'
  } else {
    npm install -g @openai/codex --no-fund --no-audit
    Update-Path
  }
  if (Get-Command codex -ErrorAction SilentlyContinue) {
    Record 'Codex CLI' $true (codex --version)
    codex login status *> $null
    if ($LASTEXITCODE -ne 0) {
      Record 'Codex 登入' $null '第一次使用：在教材資料夾開終端機輸入 codex，用 ChatGPT 帳號登入'
    }
  } else {
    Record 'Codex CLI' $false '手動執行 npm install -g @openai/codex 看錯誤訊息'
  }
}

# ─────────────────────────────────────────────
Step '9. 驗收：編譯測試（不需要接板子）'
# 這五支剛好涵蓋全部四個函式庫
$probe = 'u01-blink', 'u12-dht11', 'u17-servo', 'u18-stepper', 'u20-keypad-lock'
foreach ($name in $probe) {
  $dir = Join-Path $Course $name
  if (-not (Test-Path $dir)) { Record "編譯 $name" $null '找不到這支程式'; continue }
  & $cli compile --fqbn $FQBN $dir *> $null
  Record "編譯 $name" ($LASTEXITCODE -eq 0)
}

if (-not $ArduinoOnly -and (Test-Path (Join-Path $Workshop 'node_modules'))) {
  Step '10. 驗收：雲端程式自我測試（不需要板子，也不會真的發通知）'
  Push-Location $Workshop
  try {
    npm run selftest --silent *> $null
    Record '雲端程式自我測試' ($LASTEXITCODE -eq 0) $(if ($LASTEXITCODE -ne 0) { '手動執行 npm run selftest 看錯誤訊息' })
  } finally { Pop-Location }
}

# ─────────────────────────────────────────────
Step '11. 檢查板子（有插才會偵測到）'
$usb = @((& $cli board list --format json | ConvertFrom-Json).detected_ports |
  Where-Object { $_.port.protocol -eq 'serial' -and $_.port.properties.vid })
if ($usb.Count -ge 1) {
  Record '偵測到板子' $true (($usb | ForEach-Object { $_.port.address }) -join '、')
} else {
  # CH340 沒裝驅動時，裝置會出現在「其他裝置」，名稱通常是 USB2.0-Serial
  $noDriver = Get-PnpDevice -PresentOnly -ErrorAction SilentlyContinue |
    Where-Object { $_.FriendlyName -match 'USB2\.0-Serial|CH34' -and $_.Status -ne 'OK' }
  if ($noDriver) {
    Record '偵測到板子' $false '板子有插但缺 CH340 驅動：看上方「CH340 驅動」那一列的說明，裝好後重新插拔'
  } else {
    Record '偵測到板子' $null '沒有插板子，或 USB 線只能充電。插上後再執行一次即可確認'
  }
}

# ─────────────────────────────────────────────
Write-Host ''
Write-Host '══════════ 結果 ══════════' -ForegroundColor Cyan
$Results | Format-Table -AutoSize -Wrap | Out-String | Write-Host
$failed = @($Results | Where-Object 結果 -eq '✗').Count
if ($failed -eq 0) {
  Write-Host '全部完成，這台電腦可以上課了。' -ForegroundColor Green
  Write-Host '（標「！」的項目不影響上課，但請看一下說明。）' -ForegroundColor DarkGray
  exit 0
} else {
  Write-Host "有 $failed 項失敗，請看上方說明。修好之後重新執行，已完成的項目會自動跳過。" -ForegroundColor Red
  exit 1
}
