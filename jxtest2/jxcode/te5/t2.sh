#!/bin/bash

sudo rmmod crc2

make clean

make
sudo insmod crc2.ko

echo "success\n"


