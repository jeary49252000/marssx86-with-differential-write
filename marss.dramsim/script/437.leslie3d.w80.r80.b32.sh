#PBS -l nodes=1:ppn=1:marss
cd /home/r01/scyu/diffWrite/Wonderland/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/437.leslie3d.qcow /tmp2/MarssPower_mx/437.leslie3d.w80.r80.b32.qcow;
./script/exedir/437.leslie3d.w80.r80.b32.sh;
unlink /tmp2/MarssPower_mx/437.leslie3d.w80.r80.b32.qcow; 
