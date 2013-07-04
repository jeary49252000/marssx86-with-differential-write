# @nfs
cp /home/phd/liurs/Image/8g/429.mcf.qcow /dev/shm/sim.qcow
qemu/qemu-system-x86_64 -simconfig test.cfg  -m 8G -hda /dev/shm/sim.qcow -loadvm root.ckp -k en-us
