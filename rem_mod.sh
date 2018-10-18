#!/bin/bash

#This file is to be run as root 
if [ "$EUID" -ne 0 ]; then 
    echo "This file should be run as root. Exiting..."
    exit -1
fi

MODULE_NAME=mmdev
NUM_DEV=4

set -e

#Get major number from /proc/devices
MAJOR=$(cat /proc/devices | grep $MODULE_NAME | sed -n -e "s/^\([0-9]\+\)\s.*/\1/p")
echo "Major number is $MAJOR"
echo "Removing $MODULE_NAME devices from /dev"
rm /dev/$MODULE_NAME*

echo "Unloading module $MODULE_NAME"
rmmod $MODULE_NAME.ko
set +e
echo "DONE"


