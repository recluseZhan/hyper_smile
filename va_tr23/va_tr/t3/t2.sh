#!/bin/bash
sudo rmmod jxdev1

sudo rm -rf /dev/jxdev
make clean

make

sudo insmod jxdev1.ko
#cat /proc/devices
sudo mknod /dev/jxdev c 231 0
sudo chmod 777 /dev/jxdev
gcc jx.c
echo "success\n"


