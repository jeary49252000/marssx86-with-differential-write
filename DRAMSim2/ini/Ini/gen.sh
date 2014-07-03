#wt=( w80 w160 w320 )
wt=( w160 w320 w640 w1280)
#rd=( r40 r80 r160 )
rd=( r80 r160 )
#bk=( b1 b2 )
bk=( b8 b16 b32 )

cp=( c4 c8 c16)

for i in ${wt[*]};
do
	for j in ${rd[*]};
	do
        for k in ${bk[*]};
        do
            for l in ${cp[*]};
            do
			    cp default.ini $i'_'$j'_'$k'_'$l.ini;
            done
        done;
	done;
done;


#sed -i s/tWR=160/tWR=80/g *w80*;
sed -i s/tWR=160/tWR=320/g *w320*;
sed -i s/tWR=160/tWR=640/g *w640*;
sed -i s/tWR=160/tWR=1280/g *w1280*;


sed -i s/tRAS=80/tRAS=40/g *r40*;
sed -i s/tRCD=80/tRCD=40/g *r40*;
sed -i s/tRC=81/tRC=41/g *r40*;
sed -i s/tRTP=80/tRTP=40/g *r40*;


sed -i s/tRAS=80/tRAS=160/g *r160*;
sed -i s/tRCD=80/tRCD=160/g *r160*;
sed -i s/tRC=81/tRC=161/g *r160*;
sed -i s/tRTP=80/tRTP=160/g *r160*;

#sed -i s/NUM_BANKS=8/NUM_BANKS=1/g *_b1_*;
#sed -i s/NUM_ROWS=8388608/NUM_ROWS=67108864/g *_b1_*;

#sed -i s/NUM_BANKS=8/NUM_BANKS=2/g *b2*;
#sed -i s/NUM_ROWS=8388608/NUM_ROWS=33554432/g *_b2_*;

sed -i s/NUM_BANKS=8/NUM_BANKS=4/g *b8*;
sed -i s/NUM_ROWS=8388608/NUM_ROWS=16777216/g *b8*;
#sed -i s/NUM_ROWS=16777216/NUM_ROWS=33554432/g *b8*;

sed -i s/NUM_BANKS=8/NUM_BANKS=16/g *b32*;
sed -i s/NUM_ROWS=8388608/NUM_ROWS=4194304/g *b32*;
#sed -i s/NUM_ROWS=16777216/NUM_ROWS=8388608/g *b32*;

sed -i s/POWER_BUDGET_PER_CHIP=128/POWER_BUDGET_PER_CHIP=256/g *c16*;
sed -i s/POWER_BUDGET_PER_CHIP=128/POWER_BUDGET_PER_CHIP=64/g *c4*;
