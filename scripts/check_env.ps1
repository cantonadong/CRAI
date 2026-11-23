# Read-only environment check for CRAI: adb -> LDPlayer -> root -> Null's Royale -> libg.so mapping.
param(
    [string]$Adb = "D:\Android\sdk\platform-tools\adb.exe",
    [string]$Serial = "",            # e.g. emulator-5554 or 127.0.0.1:5555; auto-detect when empty
    [string[]]$Packages = @("nullsroyale.rel.free", "com.supercell.clashroyale")
)

$ok = $true
function Report([string]$name, [bool]$pass, [string]$detail = "") {
    $tag = if ($pass) { "[ OK ]" } else { "[FAIL]" }
    Write-Output ("{0} {1} {2}" -f $tag, $name, $detail)
    if (-not $pass) { $script:ok = $false }
}

Report "adb present" (Test-Path $Adb) $Adb
if (-not (Test-Path $Adb)) { exit 1 }

if (-not $Serial) {
    # LDPlayer9 exposes the first instance as emulator-5554 (adb port 5555).
    & $Adb connect 127.0.0.1:5555 | Out-Null
    $devs = & $Adb devices | Select-String "\tdevice$" | ForEach-Object { ($_ -split "\t")[0] }
    $Serial = @($devs | Where-Object { $_ -like "emulator-*" -or $_ -like "127.0.0.1:*" })[0]
}
Report "device connected" ([bool]$Serial) $Serial
if (-not $Serial) { Write-Output "Start LDPlayer (adbDebug=1) and retry."; exit 1 }

function Sh([string]$cmd) { (& $Adb -s $Serial shell $cmd) -join "`n" }

$abi = Sh "getprop ro.product.cpu.abilist"
$sdk = Sh "getprop ro.build.version.sdk"
Report "android" $true "sdk=$sdk abilist=$abi"

# adbd 已是 root 时直接执行，否则用 su（不主动 adb root：LDPlayer 上 root adbd 的每个 shell 都会弹提示）
$adbRoot = (Sh "id") -match "uid=0"
function RootSh([string]$cmd) { if ($adbRoot) { Sh $cmd } else { Sh "su -c '$cmd'" } }
$rootId = RootSh "id"
$isRoot = $rootId -match "uid=0"
Report "root" $isRoot $rootId.Trim()

$pkg = $null
foreach ($p in $Packages) { if ((Sh "pm path $p") -match "package:") { $pkg = $p; break } }
Report "game installed" ([bool]$pkg) "$pkg"
if ($pkg) {
    $ver = (Sh "dumpsys package $pkg") -split "`n" | Select-String "versionName|versionCode|primaryCpuAbi" | ForEach-Object { $_.ToString().Trim() }
    Report "game version" $true ($ver -join "; ")

    $gamePid = (Sh "pidof $pkg").Trim()
    Report "game running" ([bool]$gamePid) "pid=$gamePid"
    if ($gamePid -and $isRoot) {
        $maps = RootSh "grep libg.so /proc/$gamePid/maps"
        $first = ($maps -split "`n" | Select-Object -First 1)
        Report "libg.so mapped" ([bool]$first) $first.Trim()
        if ($first) {
            $path = ($first -split "\s+")[-1]
            $sha = RootSh "sha256sum $path"
            Report "libg.so sha256" $true $sha.Trim()
        }
    }
}

if ($ok) { Write-Output "All checks passed." } else { Write-Output "Some checks failed." ; exit 1 }
