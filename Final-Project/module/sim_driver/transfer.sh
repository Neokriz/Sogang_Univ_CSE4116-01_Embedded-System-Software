#! /bin/bash
clear
make clean
make
adb push sim_driver.ko /data/local/tmp
