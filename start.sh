#!/bin/bash
# Purpose: compile and upload arduino temperature control script.
# Invoke a Python script to read the temperature status from the serial port and upload it to database

cd arduino/on-off-control
make
sudo make upload

cd ..
cd ..
python python/read_serial.py

#END
