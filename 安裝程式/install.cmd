@echo off
rem Double-click to set up a student PC. Extra args pass through, e.g. install.cmd -ArduinoOnly
chcp 65001 >nul
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1" %*
echo.
pause
