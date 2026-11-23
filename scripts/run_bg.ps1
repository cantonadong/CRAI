# 后台任务启动器：由计划任务（CRAI-Train / CRAI-Ladder / CRAI-Ingest）隐藏运行，和终端、Claude 会话完全无关。
#   安装/管理：.\scripts\bg_tasks.ps1 install | start <名字> | stop <名字> | status
param([Parameter(Mandatory)][ValidateSet('train','ladder','ingest')][string]$Name)
$Root = Split-Path $PSScriptRoot -Parent
Set-Location $Root
$env:CRAI_FL_ROOT = 'yesdelay'
$env:PYTHONIOENCODING = 'utf-8'
$py = Join-Path $Root '.venv\Scripts\python.exe'
switch ($Name) {
    # run3（2026-11-23）：起点 Everlight（拿野猪卡组），联赛式对手池；先短跑 20 次更新看趋势
    'train'  { $env:CRAI_INSTANCE = '1'; $args_ = 'scripts\ppo_train.py 20 20 models/ppo/run3 refs/FirstLight_CR_yesdelay/checkpoints/Everlight/v15-general-il100-kl5e-2-v80.pt'; $out = 'models\ppo\run3\train' }
    'ladder' { $env:CRAI_INSTANCE = '0'; $args_ = 'scripts\ladder_validate.py models/ppo/run2 10 6'; $out = 'models\ppo\run2\ladder' }
    'ingest' { $env:CRAI_INSTANCE = '1'; $args_ = 'scripts\ingest_royaleapi.py replay_top200 top200 2 1'; $out = 'data\royaleapi\top200\ingest' }
}
# 旧输出追加到历史日志，避免覆盖
if (Test-Path "$out.out") { Get-Content "$out.out" -Encoding utf8 | Out-File -Append -Encoding utf8 "$out.history.log" }
$p = Start-Process $py -ArgumentList "-u $args_" -WorkingDirectory $Root -WindowStyle Hidden -PassThru `
    -RedirectStandardOutput "$out.out" -RedirectStandardError "$out.err"
$p.Id | Out-File -Encoding ascii "$out.pid"
$p.WaitForExit()
