@echo off
rem Double-click to install from the web (GitHub Pages).
chcp 65001 >nul
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { irm https://homesin.github.io/arduino-workshop/get.ps1 | iex } catch { Write-Host 'Download failed. Check your network, or download the zip from https://homesin.github.io/arduino-workshop/ ' -ForegroundColor Red }"
echo.
pause
