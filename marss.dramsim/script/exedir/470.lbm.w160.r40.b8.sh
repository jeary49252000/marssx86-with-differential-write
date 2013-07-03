#!/usr/bin/expect -f
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
spawn env SIM_DESC=470.lbm.w160.r40.b8 qemu/qemu-system-x86_64 -m 8G -simconfig simcfg/470.lbm.w160.r40.b8.cfg -drive file=/tmp2/MarssPower_mx/470.lbm.w160.r40.b8.qcow,cache=unsafe -nographic -loadvm root.ckp

set timeout 360000

sleep 5
send "\r"
expect eof
