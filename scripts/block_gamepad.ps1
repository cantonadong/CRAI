# 屏蔽/恢复模拟器里的手柄输入（避免在电脑上用手柄玩别的游戏时，模拟器里误操作）。
#   .\scripts\block_gamepad.ps1 on      编译（如需要）并在模拟器里常驻 grab_joystick（独占手柄设备）
#   .\scripts\block_gamepad.ps1 off     结束 grab_joystick，恢复手柄
#   .\scripts\block_gamepad.ps1 status  查看是否在屏蔽
# 已有常驻守护（计划任务 CRAI-GamepadGuard，scripts/gamepad_guard.ps1）自动在模拟器重启后重新 on；
# 要临时 off，先创建 build\gamepad_guard.off，否则 20 秒内会被守护重新屏蔽。
param([Parameter(Mandatory)][ValidateSet('on','off','status')][string]$Action, [string]$Serial = '')
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$AdbExe = 'D:\Android\sdk\platform-tools\adb.exe'
# 多实例：-Serial emulator-5556，或环境变量 CRAI_SERIAL / CRAI_INSTANCE（默认实例 0）
if (-not $Serial) { $Serial = $env:CRAI_SERIAL }
if (-not $Serial) { $Serial = 'emulator-' + (5554 + 2 * [int]('0' + $env:CRAI_INSTANCE)) }
function Adb { $ErrorActionPreference = 'Continue'; & $AdbExe -s $Serial @args 2>&1 | ForEach-Object { "$_" } }
$Adb = 'Adb'
Write-Host "设备 $Serial"
$Clang = 'D:\Android\sdk\ndk\27.2.12479018\toolchains\llvm\prebuilt\windows-x86_64\bin\x86_64-linux-android28-clang.cmd'
$Bin = Join-Path $Root 'build\grab_joystick'

switch ($Action) {
    'on' {
        if (-not (Test-Path $Bin)) {
            & $Clang -O2 -o $Bin (Join-Path $Root 'crai\tools\grab_joystick.c')
            if ($LASTEXITCODE) { throw 'build failed' }
        }
        & $Adb push $Bin /data/local/tmp/grab_joystick | Out-Null
        & $Adb shell "su -c 'chmod 755 /data/local/tmp/grab_joystick; pkill grab_joystick; nohup /data/local/tmp/grab_joystick > /data/local/tmp/grab_joystick.log 2>&1 &'"
        Start-Sleep 1
        & $Adb shell "cat /data/local/tmp/grab_joystick.log; pidof grab_joystick"
    }
    'off' {
        & $Adb shell "su -c 'pkill grab_joystick'"
        & $Adb shell "pidof grab_joystick" ; "已恢复手柄输入"
    }
    'status' {
        $p = (& $Adb shell "pidof grab_joystick").Trim()
        if ($p) { "正在屏蔽手柄（pid $p）" } else { "未屏蔽" }
    }
}

