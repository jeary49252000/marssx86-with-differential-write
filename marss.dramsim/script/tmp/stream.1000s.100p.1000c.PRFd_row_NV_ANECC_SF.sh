#PBS -l nodes=1:ppn=1:marss
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/stream.qcow /tmp2/MarssPower_mx/stream.1000s.100p.1000c.PRFd_row_NV_ANECC_SF.qcow;
./script/exedir/stream.1000s.100p.1000c.PRFd_row_NV_ANECC_SF.sh;
unlink /tmp2/MarssPower_mx/stream.1000s.100p.1000c.PRFd_row_NV_ANECC_SF.qcow; 
