#/usr/bin/bash

#w640.r80.b16.c8
#w1280.r160.b32.c8
#w1280.r160.b8.c8
#w1280.r160.b16.c8
Suffix4=( 
w1280.r160.b16.c8
);
Suffix3=( 
w160.r80.b8.c4
w160.r80.b8.c8
w160.r80.b8.c16
w160.r80.b16.c4
w160.r80.b16.c8
w160.r80.b16.c16
w160.r80.b32.c4
w160.r80.b32.c8
w160.r80.b32.c16
);
Suffix2=( w160.r80.b16.c8 );
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

#Benchmark=( 410.bwaves 429.mcf 470.lbm 434.zeusmp 436.cactusADM 437.leslie3d 450.soplex 459.GemsFDTD 462.libquantum 473.astar );
#Benchmark=( 410.bwaves.8c 429.mcf.8c 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c 473.astar.8c);
#Benchmark=( 410.bwaves.8c 429.mcf.8c 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c);
#Benchmark=( parsec.freqmine.8c parsec.x264.8c parsec.vips.8c parsec.facesim.8c);
#Benchmark=( 429.mcf.8c 433.milc.8c.180 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c parsec.x264.8c parsec.facesim.8c);
Benchmark=( parsec.x264.8c 433.milc.8c.180 437.leslie3d.8c parsec.facesim.8c);
#Benchmark=( parsec.x264.8c parsec.facesim.8c);
#Benchmark=( 433.milc.8c.180 437.leslie3d.8c parsec.facesim.8c);
#Benchmark=( parsec.facesim.8c);
#Benchmark=( 433.milc.8c.180 470.lbm.8c parsec.x264.8c parsec.facesim.8c);
#Benchmark=( 470.lbm.8c );
#Benchmark=( 462.libquantum.8c );
#Benchmark=( 429.mcf.8c 433.milc.8c.180 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c parsec.facesim.8c);
#Benchmark=( parsec.facesim.8c );
#Benchmark=( 429.mcf.8c 433.milc.8c.180 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c parsec.facesim.8c);
#Benchmark=( 429.mcf.8c 437.leslie3d.8c 462.libquantum.8c );
#Benchmark=( 429.mcf.8c parsec.x264.8c);
#Benchmark=( 429.mcf.8c );
#Benchmark=( 433.milc.8c.180 parsec.facesim.8c);
#Benchmark=( parsec.x264.8c );
#Benchmark=( 462.libquantum.8c parsec.facesim.8c );
#Benchmark=( 462.libquantum.8c );
#Benchmark=( 433.milc.8c.180 470.lbm.8c parsec.x264.8c parsec.facesim.8c );
#Benchmark=( 462.libquantum.8c 470.lbm.8c parsec.x264.8c );
#Benchmark=( 470.lbm.8c );
#Benchmark=( 437.leslie3d.8c );
#Benchmark=( parsec.x264.8c );
#Benchmark=(470.lbm.8c);

#Benchmark=( 433.milc.8c.60 433.milc.8c.120 433.milc.8c.180 );
#Benchmark=( 433.milc.8c.180 );
#Benchmark=( parsec.x264.8c 433.milc.8c.180);
#Benchmark=( parsec.x264.8c );

#Benchmark=( 429.mcf.8c 437.leslie3d.8c 462.libquantum.8c 470.lbm.8c);
#Benchmark=(parsec.x264.8c parsec.vips.8c parsec.freqmine.8c parsec.facesim.8c);
#Benchmark=(429.mcf.8c 462.libquantum.8c 470.lbm.8c);
#Benchmark=(429.mcf.8c);
#Benchmark=( 437.leslie3d.8c 470.lbm.8c );

#Benchmark=( 459.GemsFDTD 462.libquantum 473.astar );
#Benchmark=( 410.bwave 429.mcf 437.leslie3d 470.lbm 473.astar );
#Benchmark=( mix_4 mix_5 );
#Benchmark=( 434.zeusmp 436.cactusADM 437.leslie3d );
#Benchmark=( 410.bwaves 436.cactusADM 470.lbm mix_1 mix_2 mix_5 );
#Benchmark=( 434.zeusmp 437.leslie3d mix_3 mix_4 );
#Benchmark=( 462.libquantum.8c 470.lbm.8c parsec.x264.8c );
#Benchmark=( 429.mcf.8c 433.milc.8c.180 );

for B in ${Benchmark[*]};
do
    for S in ${Suffix4[*]};
    do
        echo $B.$S
        #qsub -q small script/$B.$S.sh
        qsub script/$B.$S.sh
        sleep 1
    done
done

