#!/bin/bash
#sudo rmmod jxdev2
#sudo rmmod limit1
sudo rmmod crc1
#sudo rm -rf /dev/jxdev
make clean

make
sudo insmod crc1.ko
#sudo insmod limit1.ko
#sudo insmod jxdev2.ko
#cat /proc/devices
#sudo mknod /dev/jxdev c 231 0
#sudo chmod 777 /dev/jxdev
echo "success\n"
