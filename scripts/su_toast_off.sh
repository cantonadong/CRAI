# 关闭 LDPlayer 内置 Superuser 的"已授予 Shell 超级用户权限"提示框（只改通知方式，不改授权日志）。
# Koush Superuser 设置：notification 0 = 不提示，1 = 提示框（默认），2 = 通知栏。恢复默认：把 0 改成 1。
# 用法（由用户执行）：adb push scripts/su_toast_off.sh /data/local/tmp/ ; adb shell "su -c 'sh /data/local/tmp/su_toast_off.sh'"
D=/data/user_de/0/com.android.settings/databases/superuser.sqlite
sqlite3 $D "insert or replace into settings(key,value) values('notification','0');"
sqlite3 $D "select * from settings;"
