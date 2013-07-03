#! /bin/bash 

#Benchmark=( 400.perlbench  416.gamess  435.gromacs    445.gobmk   454.calculix  462.libquantum  471.omnetpp  483.xalancbmk 401.bzip2      429.mcf     436.cactusADM  447.dealII  456.hmmer     464.h264ref     473.astar    998.specrand 403.gcc        433.milc    437.leslie3d   450.soplex  458.sjeng     465.tonto       481.wrf      999.specrand 410.bwaves     434.zeusmp  444.namd       453.povray  459.GemsFDTD  470.lbm         482.sphinx3 mix_1 mix_2 mix_3 mix_4 mix_5 );

Benchmark=( 473.astar 437.leslie3d );
#Benchmark=( 410.bwaves 429.mcf 433.milc 434.zeusmp 436.cactusADM 437.leslie3d 450.soplex 459.GemsFDTD 462.libquantum 470.lbm 473.astar stream mummer);


wt=( w80 w160 w320 )
rd=( r40 r80 r160 )
bk=( b8 b16 b32 )

index=0
for i in ${wt[*]};
do
	for j in ${rd[*]};
	do
		for k in ${bk[*]};
		do
					ARRAY[$index]=$i'_'$j'_'$k
					index=`expr $index + 1`
		done;
	done;
done

#retention=( 1000s 100s No );
#par=( 100p );
#Wlatency=( 1000c );

MARSS_DIRECTOR=`pwd`;
IMAGE_DIRECTOR="/tmp2/MarssPower_mx/";
IMAGE_SOURCE_DIRECTOR="/home/phd/liurs/Image/8g/";

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


echo '#!/usr/bin/expect -f
cd '$MARSS_DIRECTOR';
spawn env SIM_DESC='$m'.'$i'.'$j'.'$k' qemu/qemu-system-x86_64 -m 8G -simconfig simcfg/'$m'.'$i'.'$j'.'$k'.cfg -drive file='$IMAGE_DIRECTOR''$m'.'$i'.'$j'.'$k'.qcow,cache=unsafe -nographic -loadvm root.ckp

set timeout 360000

sleep 5
send "\r"
expect eof' > script/exedir/$m.$i.$j.$k.sh ;

echo '#PBS -l nodes=1:ppn=1:marss
cd '$MARSS_DIRECTOR';
if [ ! -d "'$IMAGE_DIRECTOR'" ]; then
	mkdir '$IMAGE_DIRECTOR'
fi
cp '$IMAGE_SOURCE_DIRECTOR''$m'.qcow '$IMAGE_DIRECTOR''$m'.'$i'.'$j'.'$k'.qcow;
./script/exedir/'$m'.'$i'.'$j'.'$k'.sh;
unlink '$IMAGE_DIRECTOR''$m'.'$i'.'$j'.'$k'.qcow; ' > script/$m.$i.$j.$k.sh ;



# simconfig script
echo '-corefreq 4000000000
-machine private_L3
-stopinsns 1501000000
-kill-after-run
-logfilesize 268435456
-logfile log/'$m'.'$i'.'$j'.'$k'.log
-dramsim-device-ini-file ini/Ini/'$i'_'$j'_'$k'.ini
-dramsim-pwd ../DRAMSim2
-dramsim-system-ini-file ini/System/default.ini
-dramsim-results-dir-name log
-run true' > simcfg/$m.$i.$j.$k.cfg;
												done;
											done;
                                        done;
                                    done;
								chmod +x script/*.sh 
								chmod +x script/exedir/*.sh
