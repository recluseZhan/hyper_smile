#!/bin/bash
cd /home/vjxzhan/sgx-step/app/smile-case1
i=1
while [ $i -le 100 ]
do
    sudo ./app
    i=$((i+1))
done
echo "$((i-1)) times finished."
