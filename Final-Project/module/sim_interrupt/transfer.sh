#! /bin/bash
clear
make clean
make
adb push sim_interrupt.ko /data/local/tmp
