#!/bin/bash
sudo rmmod check1
make clean

make
sudo insmod check1.ko
echo "success\n"


