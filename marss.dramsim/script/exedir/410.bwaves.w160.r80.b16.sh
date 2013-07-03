#!/usr/bin/expect -f
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
spawn env SIM_DESC=410.bwaves.w160.r80.b16 qemu/qemu-system-x86_64 -m 8G -simconfig simcfg/410.bwaves.w160.r80.b16.cfg -drive file=/tmp2/MarssPower_mx/410.bwaves.w160.r80.b16.qcow,cache=unsafe -nographic -loadvm root.ckp

set timeout 360000

sleep 5
send "\r"
expect eof
