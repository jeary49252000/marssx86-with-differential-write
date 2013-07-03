#PBS -l nodes=1:ppn=1:marss
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/473.astar.qcow /tmp2/MarssPower_mx/473.astar.w160.r160.b16.qcow;
./script/exedir/473.astar.w160.r160.b16.sh;
unlink /tmp2/MarssPower_mx/473.astar.w160.r160.b16.qcow; 
