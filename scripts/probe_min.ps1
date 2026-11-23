# 最小 probe 的编译、部署、回滚。
#   .\scripts\probe_min.ps1 build     编译 build/probe/libsentry-android.so
#   .\scripts\probe_min.ps1 deploy    停游戏，备份原库为 libsentry-android-real.so，换上代理库
#   .\scripts\probe_min.ps1 restore   停游戏，放回原库，删除备份
#   .\scripts\probe_min.ps1 log       看 CRProbe 日志
#   多实例：-Serial emulator-5556，或设置环境变量 CRAI_SERIAL / CRAI_INSTANCE（默认实例 0 = emulator-5554）
param([Parameter(Mandatory)][ValidateSet('build','deploy','restore','log')][string]$Action, [string]$Serial = '')
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$AdbExe = 'D:\Android\sdk\platform-tools\adb.exe'
if (-not $Serial) { $Serial = $env:CRAI_SERIAL }
if (-not $Serial) { $Serial = 'emulator-' + (5554 + 2 * [int]('0' + $env:CRAI_INSTANCE)) }
# adb 把进度写到 stderr；PowerShell 5.1 在 Stop 模式下会把它当成错误，所以函数内用 Continue
function Adb { $ErrorActionPreference = 'Continue'; & $AdbExe -s $Serial @args 2>&1 | ForEach-Object { "$_" } }
$Adb = 'Adb'
Write-Host "设备 $Serial"
$Pkg = 'nullsroyale.rel.free'
$Clang = 'D:\Android\sdk\ndk\27.2.12479018\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android28-clang.cmd'
$Out = Join-Path $Root 'build\probe_min'

# 需要 root 的命令：adbd 已是 root 时直接执行，否则用 su（不主动 adb root：root adbd 的每个 shell 都会弹提示）
function Invoke-Root([string]$cmd) {
    if ((& $Adb shell id -u).Trim() -eq '0') { & $Adb shell $cmd } else { & $Adb shell "su -c '$cmd'" }
}

function Get-LibDir {
    $p = (& $Adb shell "pm path $Pkg" | Select-Object -First 1).Trim() -replace '^package:', ''
    return ($p -replace '/base\.apk$', '') + '/lib/arm64'
}

switch ($Action) {
    'build' {
        New-Item -ItemType Directory -Force $Out | Out-Null
        # 链接时让 DT_NEEDED 指向改名后的原库
        Copy-Item (Join-Path $Root 'apk\libs\arm64-v8a\libsentry-android.so') (Join-Path $Out 'libsentry-android-real.so') -Force
        & $Clang -shared -fPIC -O2 -Wall -o (Join-Path $Out 'libsentry-android.so') `
            (Join-Path $Root 'crai\probe\proxy_min.c') (Join-Path $Root 'crai\probe\native_call_arm64.S') `
            "-Wl,-soname,libsentry-android.so" "-L$Out" "-l:libsentry-android-real.so" -llog
        if ($LASTEXITCODE) { throw 'build failed' }
        Get-Item (Join-Path $Out 'libsentry-android.so') | Select-Object Name, Length
    }
    'deploy' {
        $d = Get-LibDir
        & $Adb push (Join-Path $Out 'libsentry-android.so') /data/local/tmp/crprobe.so | Out-Null
        Invoke-Root "am force-stop $Pkg; cd $d; [ -f libsentry-android-real.so ] || cp -p libsentry-android.so libsentry-android-real.so; cp /data/local/tmp/crprobe.so libsentry-android.so; chmod 755 libsentry-android.so; chown system:system libsentry-android.so; ls -laZ"
    }
    'restore' {
        $d = Get-LibDir
        Invoke-Root "am force-stop $Pkg; cd $d; [ -f libsentry-android-real.so ] && cp -p libsentry-android-real.so libsentry-android.so && rm libsentry-android-real.so; ls -la"
    }
    'log' {
        & $Adb logcat -d -s CRProbe
    }
}
