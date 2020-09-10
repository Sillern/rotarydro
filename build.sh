#!/bin/bash
mkdir build
cd build
cmake -D CMAKE_TOOLCHAIN_FILE=../../../Arduino-CMake-Toolchain/Arduino-toolchain.cmake -DARDUINO_BOARD=avr.nano -DARDUINO_AVR_NANO_MENU_CPU_ATMEGA328OLD=TRUE ..
make
cd ..
