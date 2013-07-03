#PBS -l nodes=1:ppn=1:marss
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/429.mcf.qcow /tmp2/MarssPower_mx/429.mcf.w160.r160.b16.qcow;
./script/exedir/429.mcf.w160.r160.b16.sh;
unlink /tmp2/MarssPower_mx/429.mcf.w160.r160.b16.qcow; 
