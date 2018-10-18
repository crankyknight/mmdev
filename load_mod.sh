#!/bin/bash

#This file is to be run as root 
if [ "$EUID" -ne 0 ]; then 
    echo "This file should be run as root. Exiting..."
    exit -1
fi
#Run make 
make

MODULE_NAME=mmdev
NUM_DEV=4

set -e
echo "Loading module $MODULE_NAME"
insmod $MODULE_NAME.ko

#Get major number from /proc/devices
MAJOR=$(cat /proc/devices | grep $MODULE_NAME | sed -n -e "s/^\([0-9]\+\)\s.*/\1/p")
echo "Major number is $MAJOR"
echo "Adding $NUM_DEV devices to /dev"

for ((i=0; i<$NUM_DEV; i++)); do
    mknod -m 0666 /dev/mmdev$i c $MAJOR $i
done
set +e
echo "DONE"


