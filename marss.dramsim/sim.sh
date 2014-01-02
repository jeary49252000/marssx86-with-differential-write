# @nfs
#cp /home/phd/liurs/Image/8g/429.mcf.qcow /dev/shm/sim3.qcow
#cp /tmp3/scyu_marss/470.lbm.multi.qcow /dev/shm/sim.qcow
#cp /tmp3/scyu_marss/473.astar.8c.qcow /dev/shm/sim.qcow
#cp /tmp3/scyu_marss/470.lbm.8c.qcow /dev/shm/sim.qcow
cp /tmp3/scyu_marss/429.mcf.8c.qcow /dev/shm/sim.qcow
#cp /tmp3/scyu_marss/462.libquantum.qcow /dev/shm/sim.qcow
#cp /tmp3/scyu_marss/debian.cpu2006.qcow2 /tmp3/scyu_marss/tmp.qcow
#cp /tmp3/scyu_marss/debian.cpu2006m.qcow2 /tmp3/scyu_marss/1.qcow

#cp /home/phd/liurs/Image/8g/437.leslie3d.qcow /dev/shm/sim.qcow
qemu/qemu-system-x86_64 -simconfig test.cfg  -m 8G -hda /dev/shm/sim.qcow -loadvm root.ckp -k en-us
#qemu/qemu-system-x86_64 -simconfig test.cfg  -m 8G -hda /tmp3/scyu_marss/1.qcow -loadvm libquantum -k en-us
#qemu/qemu-system-x86_64 -simconfig test.cfg  -m 8G -hda /tmp3/scyu_marss/tmp.qcow -k en-us
