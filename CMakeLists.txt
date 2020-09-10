cmake_minimum_required(VERSION 3.0.0)

project(rotary_dro CXX)

add_executable(${PROJECT_NAME} rotary_dro.cpp)

# Link with EEPROM native library, because arduino_lib.cpp includes EEPROM.h
# Link with Stepper library, because arduino_lib.cpp includes Stepper.h
# Also link with core, because arduino_lib.cpp includes Arduino.h
target_link_arduino_libraries(${PROJECT_NAME} PRIVATE EEPROM Wire LiquidCrystal_I2C Encoder Keypad core)

# This is needed for generation of HEX image and uploading it
target_enable_arduino_upload(${PROJECT_NAME})
