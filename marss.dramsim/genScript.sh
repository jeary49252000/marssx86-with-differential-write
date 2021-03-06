#! /bin/bash

#Benchmark=( 400.perlbench  416.gamess  435.gromacs    445.gobmk   454.calculix  462.libquantum  471.omnetpp  483.xalancbmk 401.bzip2      429.mcf     436.cactusADM  447.dealII  456.hmmer     464.h264ref     473.astar    998.specrand 403.gcc        433.milc    437.leslie3d   450.soplex  458.sjeng     465.tonto       481.wrf      999.specrand 410.bwaves     434.zeusmp  444.namd       453.povray  459.GemsFDTD  470.lbm         482.sphinx3 mix_1 mix_2 mix_3 mix_4 mix_5 );

#Benchmark=( 473.astar 437.leslie3d );
#Benchmark=( 410.bwaves 434.zeusmp 437.leslie3d 470.lbm 473.astar );
#Benchmark=( 410.bwaves 429.mcf 433.milc 434.zeusmp 436.cactusADM 437.leslie3d 450.soplex 459.GemsFDTD 462.libquantum 470.lbm 473.astar stream mummer);
#Benchmark=( 410.bwaves.8c 429.mcf.8c 433.milc.8c 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c 459.gems.8c parsec.freqmine.8c parsec.x264.8c parsec.vips.8c parsec.facesim.8c 433.milc.8c.180);

Benchmark=( 429.mcf.8c 433.milc.8c.180 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c parsec.x264.8c parsec.facesim.8c );
#Benchmark=( 429.mcf.8c 433.milc.8c.180 470.lbm.8c parsec.x264.8c parsec.facesim.8c );
#Benchmark=( 462.libquantum.8c parsec.facesim.8c );
#Benchmark=( 462.libquantum.8c );
#Benchmark=( 462.libquantum.8c 470.lbm.8c parsec.x264.8c );
#Benchmark=( parsec.x264.8c );
#Benchmark=( 433.milc.8c.180 470.lbm.8c parsec.x264.8c parsec.facesim.8c );
#Benchmark=( 462.libquantum.8c 470.lbm.8c parsec.x264.8c );
#Benchmark=( 470.lbm.8c );
#Benchmark=( 437.leslie3d.8c parsec.x264.8c );
#Benchmark=( 433.milc.8c.60 433.milc.8c.120 433.milc.8c.180 );

abs_path=/home/bachelor/laisky/PCM_rerun/marssx86-with-differential-write/

#wt=( w80 w160 w320 )
wt=( w160 w320 w640 w1280)
#rd=( r40 r80 r160 )
rd=(r80 r160)
bk=( b8 b16 b32 )
#bk=(b16)

#cp=(c4 c8 c16)
cp=(c8)

index=0
for i in ${wt[*]};
do
	for j in ${rd[*]};
	do
		for k in ${bk[*]};
		do
		    for l in ${cp[*]};
		    do
					ARRAY[$index]=$i'_'$j'_'$k'_'$l
					index=`expr $index + 1`
            done
		done;
	done;
done

#retention=( 1000s 100s No );
#par=( 100p );
#Wlatency=( 1000c );

MARSS_DIRECTOR=`pwd`;
IMAGE_DIRECTOR="/tmp3/laisky/";
IMAGE_SOURCE_DIRECTOR="/tmp3/laisky/";
#IMAGE_SOURCE_DIRECTOR="/home/phd/liurs/Image/8g/";

# check directory
if [ ! -d "script" ]; then
	mkdir script
fi

if [ ! -d "script/exedir" ]; then
	mkdir script/exedir
fi

if [ ! -d "log" ]; then
	mkdir log
fi

if [ ! -d "simcfg" ]; then
	mkdir simcfg
fi


for m in ${Benchmark[*]};
do
    for i in ${wt[*]};
    do
        for j in ${rd[*]};
        do
            for k in ${bk[*]};
            do
                for l in ${cp[*]};
                do


echo '#!/usr/bin/expect -f
cd '$MARSS_DIRECTOR';
spawn env SIM_DESC='$m'.'$i'.'$j'.'$k'.'$l' qemu/qemu-system-x86_64 -m 8G -simconfig simcfg/'$m'.'$i'.'$j'.'$k'.'$l'.cfg -drive file='$IMAGE_DIRECTOR''$m'.'$i'.'$j'.'$k'.'$l'.qcow,cache=unsafe -nographic -loadvm root.ckp

set timeout 360000

sleep 5
send "\r"
expect eof' > script/exedir/$m.$i.$j.$k.$l.sh ;

echo '#PBS -l nodes=1:ppn=1:scyu
cd '$MARSS_DIRECTOR';
if [ ! -d "'$IMAGE_DIRECTOR'" ]; then
	mkdir '$IMAGE_DIRECTOR'
fi
cp '$IMAGE_SOURCE_DIRECTOR''$m'.qcow '$IMAGE_DIRECTOR''$m'.'$i'.'$j'.'$k'.'$l'.qcow;
'$abs_path'/marss.dramsim/script/exedir/'$m'.'$i'.'$j'.'$k'.'$l'.sh;
unlink '$IMAGE_DIRECTOR''$m'.'$i'.'$j'.'$k'.'$l'.qcow; ' > script/$m.$i.$j.$k.$l.sh ;

# -machine private_L3
# -stopinsns 1501000000
# -dramsim-system-ini-file ini/System/default.ini
# -dramsim-system-ini-file ini/System/budget_schedule.ini
# simconfig script
#TYPE='default'
#TYPE='fwc'
#TYPE='budget'
TYPE='budget_schedule'
#ATTR='scheme6.r2.col64.sub.dynamic.issuemore'
#ATTR='r2.sub.dynamic'
#ATTR='r2.sub.dynamic.nB.trackDynamic'
#ATTR='r2.nosub.static.nB.BaseLine'
#ATTR='r2.nosub.static.nB.BaseLine.Q36'
#ATTR='r2.nosub.static.nB.BaseLine.TracePU'
#ATTR='r2.nosub.static.nB.BaseLine.TracePU17'
#ATTR='r2.nosub.static.nB.FWC.changeFlag'
#ATTR='r2.nosub.static.nB.FWC.changeFlag.TracePU'
#ATTR='r2.nosub.static.nB.FWC.changeFlag.TracePU17'
#ATTR='r2.sub.static.nB.DivisionSch'
#ATTR='r2.sub.static.nB.DivisionSch.TracePU'
#ATTR='r2.sub.static.nB.DivisionSch.TracePU13'
ATTR='r2.sub.static.nB.DivisionSch.TracePU17'
#ATTR='r2.sub.dynamic.nB.DivisionDyn'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.TracePU3'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.TracePU17'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.NoIssueMore'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.SmallestIndex'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.AddDelay'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.GETPU01'
#ATTR='r2.sub.dynamic.nB.DivisionDyn.Q36'
#ATTR='r2.nosub.static.nB.UnLimited'
#ATTR='r2.sub.dynamic.BC'
#ATTR='r2.sub.dynamic.Token'
#ATTR='r2.nosub.dynamic'
#ATTR='r2.nosub.dynamic.BC'
#ATTR='r2.nosub.static'
#ATTR='r2.nosub.static.newBaseLine'
#ATTR='r2.sub.static.deBug'
#ATTR='r2.sub.static.BC'
#ATTR='r2.sub.static.Token'
echo '-corefreq 4000000000
-machine private_L3
-stopinsns 1501000000
-kill-after-run
-logfilesize 268435456
-logfile '$abs_path'/marss.dramsim/log/'$m'.'$i'.'$j'.'$k'.'$l'.log.8c.'$ATTR'.'$TYPE'
-dramsim-device-ini-file ini/Ini/'$i'_'$j'_'$k'_'$l'.ini
-dramsim-pwd '$abs_path'/DRAMSim2
-dramsim-system-ini-file ini/System/'$TYPE'.ini
-dramsim-results-dir-name '$abs_path'/marss.dramsim/log/dramsim.'$m'_'$i'_'$j'_'$k'_'$l'.log.8c.'$ATTR'.'$TYPE'
-run true' > simcfg/$m.$i.$j.$k.$l.cfg;
                                                    done;
												done;
											done;
                                        done;
                                    done;
								chmod +x script/*.sh 
								chmod +x script/exedir/*.sh
