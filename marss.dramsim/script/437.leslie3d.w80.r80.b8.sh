#PBS -l nodes=1:ppn=1:marss
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/437.leslie3d.qcow /tmp2/MarssPower_mx/437.leslie3d.w80.r80.b8.qcow;
./script/exedir/437.leslie3d.w80.r80.b8.sh;
unlink /tmp2/MarssPower_mx/437.leslie3d.w80.r80.b8.qcow; 
