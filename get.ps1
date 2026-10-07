# Arduino workshop - web installer, stage 1.
#
#   irm https://homesin.github.io/arduino-workshop/get.ps1 | iex
#
# Downloads the course zip, extracts it to a temp folder, then hands over to
# setup\from-web.ps1 inside the zip (which copies it to the Desktop and runs install.ps1).
#
# ASCII only on purpose: nginx serves .ps1 as application/octet-stream without a charset,
# so any Chinese text here would reach the student garbled. Chinese messages live in from-web.ps1.
# deploy.ps1 fills in https://homesin.github.io/arduino-workshop and $false and writes this file twice (get.ps1 / get-arduino.ps1).
& {
  $ErrorActionPreference = 'Stop'
  $ProgressPreference = 'SilentlyContinue'   # Invoke-WebRequest is ~10x slower with the progress bar on 5.1
  [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12

  $base = 'https://homesin.github.io/arduino-workshop'
  $only = $false
  $tmp  = Join-Path $env:TEMP ('arduino-workshop-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
  $zip  = Join-Path $tmp 'arduino-workshop.zip'
  $src  = Join-Path $tmp 'src'
  New-Item -ItemType Directory -Force $tmp | Out-Null

  try {
    Write-Host "Downloading $base/arduino-workshop.zip ..." -ForegroundColor Cyan
    Invoke-WebRequest "$base/arduino-workshop.zip" -OutFile $zip -UseBasicParsing
    Expand-Archive $zip $src -Force
    Get-ChildItem $src -Recurse -File | Unblock-File
  } catch {
    Write-Host "Download failed: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Please use the zip download on $base/ instead." -ForegroundColor Yellow
    Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
    return
  }

  # A separate process: install.ps1 calls exit on failure, which would otherwise close this window.
  $a = '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $src 'setup\from-web.ps1'), '-Source', $src
  if ($only) { $a += '-ArduinoOnly' }
  & powershell.exe @a
  Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
}
