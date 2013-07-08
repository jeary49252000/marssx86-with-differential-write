#/usr/bin/bash

Suffix2=( w160.r80.b16 );
Suffix1=( w160.r40.b16
w160.r160.b16
w160.r80.b8
w160.r80.b32
w80.r80.b16
w320.r80.b16
);


#100s.100p.1000c.PRFd_row_V_ANECC_SF  

#Suffix=( No.100p.1000c.RFd_row_V_ECC_NSF 1000s.100p.1000c.PRFd_row_V_ECC_NSF 1000s.100p.1000c.PRFd_row_V_ECC_SF \
#1000s.100p.1000c.RFd_row_V_ECC_NSF 1000s.100p.1000c.RFd_line_V_ECC_NSF );

#Benchmark=( 434.zeusmp 437.leslie3d 459.GemsFDTD 473.astar );
#Benchmark=( 410.bwaves 429.mcf 436.cactusADM 470.lbm 473.astar 437.lieslie3d);
#Benchmark=( 410.bwaves );
Benchmark=( 410.bwaves 434.zeusmp 437.leslie3d 470.lbm 473.astar );
#Benchmark=( 410.bwaves 429.mcf 470.lbm 434.zeusmp 436.cactusADM 437.leslie3d 450.soplex 459.GemsFDTD 462.libquantum 473.astar mummer stream );
#Benchmark=( 436.cactusADM );
#Benchmark=( 410.bwave 429.mcf 437.leslie3d 470.lbm 473.astar );
#Benchmark=( mix_4 mix_5 );
#Benchmark=( 434.zeusmp 436.cactusADM 437.leslie3d );
#Benchmark=( 410.bwaves 436.cactusADM 470.lbm mix_1 mix_2 mix_5 );
#Benchmark=( 434.zeusmp 437.leslie3d mix_3 mix_4 );

for B in ${Benchmark[*]};
do
    # baseline
    qsub script/$B.w160.r80.b16.sh
    sleep 1
    for S in ${Suffix1[*]};
    do
        echo $B.$S
        qsub script/$B.$S.sh
        sleep 1
    done
done

