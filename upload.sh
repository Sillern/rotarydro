#!/bin/bash
mkdir build
cd build
make upload-rotary_dro SERIAL_PORT=/dev/ttyUSB0
cd ..
