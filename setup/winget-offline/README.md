# winget 離線安裝檔

`install.ps1` 在找不到 winget 的電腦上會自動補裝。它會先找這個資料夾，沒有才從 GitHub 下載（約 300 MB，學校網路有時很慢）。

電腦教室要一次裝很多台時，先在一台電腦下載這兩個檔案放進來，再把整個教材資料夾用隨身碟或網路磁碟發出去：

- `DesktopAppInstaller_Dependencies.zip`
- `Microsoft.DesktopAppInstaller_8wekyb3d8bbwe.msixbundle`

下載位置：<https://github.com/microsoft/winget-cli/releases/latest>（在 Assets 底下）

大部分 Windows 10／11 本來就有 winget，用不到這個資料夾。
這兩個檔案不進版控（見 `.gitignore`）。
