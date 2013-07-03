#PBS -l nodes=1:ppn=1:marss
cd /home/phd/liurs/task/MarssDramsim/marss.dramsim;
if [ ! -d "/tmp2/MarssPower_mx/" ]; then
	mkdir /tmp2/MarssPower_mx/
fi
cp /home/phd/liurs/Image/8g/436.cactusADM.qcow /tmp2/MarssPower_mx/436.cactusADM.No.100p.1000c.RFd_row_V_ANECC_NSF.qcow;
./script/exedir/436.cactusADM.No.100p.1000c.RFd_row_V_ANECC_NSF.sh;
unlink /tmp2/MarssPower_mx/436.cactusADM.No.100p.1000c.RFd_row_V_ANECC_NSF.qcow; 
