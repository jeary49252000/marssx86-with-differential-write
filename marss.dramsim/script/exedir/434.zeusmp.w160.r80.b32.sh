#!/usr/bin/expect -f
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
spawn env SIM_DESC=434.zeusmp.w160.r80.b32 qemu/qemu-system-x86_64 -m 8G -simconfig simcfg/434.zeusmp.w160.r80.b32.cfg -drive file=/tmp2/MarssPower_mx/434.zeusmp.w160.r80.b32.qcow,cache=unsafe -nographic -loadvm root.ckp

set timeout 360000

sleep 5
send "\r"
expect eof
