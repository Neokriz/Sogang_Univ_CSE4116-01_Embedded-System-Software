#! /bin/bash
make clean
make
adb push stopwatch_driver.ko /data/local/tmp
