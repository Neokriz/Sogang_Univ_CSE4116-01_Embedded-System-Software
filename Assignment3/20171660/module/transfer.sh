#! /bin/bash
clear
make clean
make
adb push stopwatch.ko /data/local/tmp
