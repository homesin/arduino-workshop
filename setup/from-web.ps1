# 網頁一行指令安裝的第二段：get.ps1 把教材下載、解壓到暫存資料夾後呼叫這支。
# 把教材放到桌面的 arduino-workshop\，再執行 install.ps1。
#
# 桌面已經有這個資料夾（重跑、或講師更新了教材）時：setup\ 一律換成新版，
# 其餘只補上缺少的檔案 —— 學員改過的程式和填好的 .env 不會被蓋掉。

param(
  [Parameter(Mandatory)][string]$Source,
  [switch]$ArduinoOnly
)

[Console]::OutputEncoding = [Text.Encoding]::UTF8
$ProgressPreference = 'SilentlyContinue'

$Source = $Source.TrimEnd('\')
$Dest   = Join-Path ([Environment]::GetFolderPath('Desktop')) 'arduino-workshop'

# 用 robocopy，不要自己用字串長度切相對路徑：帳號名稱超過 8 個字元時 %TEMP% 是 8.3 短檔名
# （C:\Users\WDAGUT~1\…），Get-ChildItem 卻回傳長檔名，兩邊長度對不上，切出來的路徑全錯。
function Sync($from, $to, [string[]]$opts) {
  robocopy $from $to /E /NFL /NDL /NJH /NJS /NP @opts | Out-Null
  if ($LASTEXITCODE -ge 8) { Write-Host "  ✗ 複製失敗（robocopy 結束碼 $LASTEXITCODE）" -ForegroundColor Red; exit 1 }
}

Write-Host ''
if (-not (Test-Path (Join-Path $Dest 'setup\install.ps1'))) {
  Write-Host "▶ 把教材放到 $Dest" -ForegroundColor Cyan
} else {
  Write-Host "▶ 桌面已經有 arduino-workshop，只更新安裝腳本、補上缺少的檔案（你改過的程式不會被蓋掉）" -ForegroundColor Cyan
}
# /XC /XN /XO：已經存在的檔案一律不動，只補缺少的；接著 setup\ 整個換成新版
Sync $Source $Dest '/XC', '/XN', '/XO'
Sync (Join-Path $Source 'setup') (Join-Path $Dest 'setup')

# 另開一個 powershell 跑：install.ps1 失敗時會 exit，不要連這支一起結束
$a = '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $Dest 'setup\install.ps1')
if ($ArduinoOnly) { $a += '-ArduinoOnly' }
& powershell.exe @a

Write-Host ''
Write-Host '教材在桌面的 arduino-workshop 資料夾。' -ForegroundColor Green
Write-Host '之後要重新檢查（例如插上板子後），雙擊裡面的 setup\install.cmd 即可。' -ForegroundColor DarkGray
Start-Process explorer.exe $Dest
