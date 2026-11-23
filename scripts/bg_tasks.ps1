# 管理后台长任务（Windows 计划任务，关掉终端 / 退出 Claude 都不影响）。
#   .\scripts\bg_tasks.ps1 install            注册 CRAI-Train / CRAI-Ladder / CRAI-Ingest（按需启动，失败自动重启）
#   .\scripts\bg_tasks.ps1 start  train       启动（train | ladder | ingest）
#   .\scripts\bg_tasks.ps1 stop   train       停止（连同 python 子进程）
#   .\scripts\bg_tasks.ps1 status             查看
# 日志：models\ppo\run2\train.out / ladder.out，data\royaleapi\top200\ingest.out
param([Parameter(Mandatory)][ValidateSet('install','start','stop','status')][string]$Action, [string]$Name = '')
$Tasks = @{ train = 'CRAI-Train'; ladder = 'CRAI-Ladder'; ingest = 'CRAI-Ingest' }
$Match = @{ train = 'ppo_train'; ladder = 'ladder_validate'; ingest = 'ingest_royaleapi' }
$Runner = Join-Path $PSScriptRoot 'run_bg.ps1'

function Stop-One($n) {
    Stop-ScheduledTask -TaskName $Tasks[$n] -ErrorAction SilentlyContinue
    Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match $Match[$n] -or ($_.CommandLine -match 'run_bg\.ps1' -and $_.CommandLine -match "-Name $n") } |
        ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    # 采集子进程（multiprocessing spawn）父进程死后可能残留：按父进程已不存在的 spawn_main 清理（只清本项目 python）
    if ($n -eq 'train') {
        Start-Sleep 3
        Get-CimInstance Win32_Process -Filter "Name='python.exe'" | Where-Object { $_.CommandLine -match 'spawn_main' } | ForEach-Object {
            if (-not (Get-Process -Id $_.ParentProcessId -ErrorAction SilentlyContinue)) { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
        }
    }
}

switch ($Action) {
    'install' {
        foreach ($n in $Tasks.Keys) {
            $act = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$Runner`" -Name $n"
            $set = New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::Zero) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries `
                -RestartCount 20 -RestartInterval (New-TimeSpan -Minutes 2) -MultipleInstances IgnoreNew
            Register-ScheduledTask -TaskName $Tasks[$n] -Action $act -Settings $set -Force | Out-Null
            "已注册 $($Tasks[$n])"
        }
    }
    'start' { Start-ScheduledTask -TaskName $Tasks[$Name]; "已启动 $($Tasks[$Name])" }
    'stop'  { Stop-One $Name; "已停止 $Name" }
    'status' {
        foreach ($n in $Tasks.Keys) {
            $t = Get-ScheduledTask -TaskName $Tasks[$n] -ErrorAction SilentlyContinue
            $py = @(Get-CimInstance Win32_Process -Filter "Name='python.exe'" | Where-Object { $_.CommandLine -match $Match[$n] }).Count
            "{0,-12} 任务状态 {1,-8} python 进程 {2}" -f $Tasks[$n], $(if ($t) { $t.State } else { '未注册' }), $py
        }
    }
}
