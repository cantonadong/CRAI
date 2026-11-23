# 永久屏蔽模拟器的手柄：常驻守护，每 20 秒检查所有在线的 LDPlayer 实例，
# 没在跑 grab_joystick 的就补上（模拟器重启、新开实例都会自动屏蔽）。
# 由计划任务 "CRAI-GamepadGuard" 在登录 Windows 时隐藏启动（见 scripts/install_gamepad_guard.ps1）。
# 临时想在模拟器里用手柄：创建文件 build\gamepad_guard.off，再 .\scripts\block_gamepad.ps1 off；删掉该文件即恢复屏蔽。
$ErrorActionPreference = 'Continue'
$Root = Split-Path $PSScriptRoot -Parent
$AdbExe = 'D:\Android\sdk\platform-tools\adb.exe'
$Bin = Join-Path $Root 'build\grab_joystick'
$Off = Join-Path $Root 'build\gamepad_guard.off'
$Log = Join-Path $Root 'build\gamepad_guard.log'
function Write-Log($m) { "$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') $m" | Out-File -Append -Encoding utf8 $Log }

# 单实例：已有守护在跑就退出
$me = $PID
$others = Get-CimInstance Win32_Process -Filter "Name='powershell.exe'" |
    Where-Object { $_.ProcessId -ne $me -and $_.CommandLine -match 'gamepad_guard\.ps1' }
if ($others) { exit 0 }
Write-Log "守护启动 pid $me"

while ($true) {
    if (-not (Test-Path $Off)) {
        $devs = & $AdbExe devices 2>$null | Where-Object { $_ -match '^(emulator-\d+)\s+device$' } | ForEach-Object { $Matches[1] }
        foreach ($s in $devs) {
            $p = "$(& $AdbExe -s $s shell 'pidof grab_joystick' 2>$null)".Trim()
            if ($p) { continue }
            $has = "$(& $AdbExe -s $s shell 'ls /data/local/tmp/grab_joystick' 2>$null)".Trim()
            if ($has -notmatch 'grab_joystick$') { & $AdbExe -s $s push $Bin /data/local/tmp/grab_joystick 2>$null | Out-Null }
            & $AdbExe -s $s shell "su -c 'chmod 755 /data/local/tmp/grab_joystick; nohup /data/local/tmp/grab_joystick > /data/local/tmp/grab_joystick.log 2>&1 &'" 2>$null | Out-Null
            Start-Sleep 1
            $r = "$(& $AdbExe -s $s shell 'cat /data/local/tmp/grab_joystick.log' 2>$null)".Trim()
            Write-Log "$s 补屏蔽：$r"
        }
    }
    Start-Sleep 20
}
