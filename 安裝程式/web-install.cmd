@echo off
rem Double-click to install from the web. Tries GitHub Pages first, then the nginx fallback.
chcp 65001 >nul
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { irm https://homesin.github.io/arduino-workshop/get.ps1 | iex } catch { Write-Host 'GitHub Pages failed, trying backup...' -ForegroundColor Yellow; irm https://homeshin.ddns.net/web/course/arduino/get.ps1 | iex }"
echo.
pause