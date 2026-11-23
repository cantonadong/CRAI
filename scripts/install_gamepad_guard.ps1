# 安装/卸载"永久屏蔽模拟器手柄"的守护（计划任务，登录 Windows 时隐藏启动 gamepad_guard.ps1）。
#   .\scripts\install_gamepad_guard.ps1          安装并立即启动
#   .\scripts\install_gamepad_guard.ps1 remove   卸载并停止
param([string]$Action = 'install')
$Name = 'CRAI-GamepadGuard'
$Script = Join-Path $PSScriptRoot 'gamepad_guard.ps1'
if ($Action -eq 'remove') {
    Unregister-ScheduledTask -TaskName $Name -Confirm:$false -ErrorAction SilentlyContinue
    Get-CimInstance Win32_Process -Filter "Name='powershell.exe'" | Where-Object { $_.CommandLine -match 'gamepad_guard\.ps1' } |
        ForEach-Object { Stop-Process -Id $_.ProcessId -Force }
    "已卸载"; return
}
$act = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$Script`""
$trg = New-ScheduledTaskTrigger -AtLogOn -User $env:USERNAME
$set = New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::Zero) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -RestartCount 99 -RestartInterval (New-TimeSpan -Minutes 1)
Register-ScheduledTask -TaskName $Name -Action $act -Trigger $trg -Settings $set -Force | Out-Null
Start-ScheduledTask -TaskName $Name
"已安装并启动计划任务 $Name"
