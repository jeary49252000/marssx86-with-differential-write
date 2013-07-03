#PBS -l nodes=1:ppn=1:marss
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/mummer.qcow /tmp2/MarssPower_mx/mummer.No.100p.1000c.RFd_row_NV_ANECC_SF.qcow;
./script/exedir/mummer.No.100p.1000c.RFd_row_NV_ANECC_SF.sh;
unlink /tmp2/MarssPower_mx/mummer.No.100p.1000c.RFd_row_NV_ANECC_SF.qcow; 
