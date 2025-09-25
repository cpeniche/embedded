# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(driverModule)
set(TARGET_NAME app)
set(sources src/main.cpp
            src/tle94103.cpp
            src/button.cpp
            src/dictionary.cpp
            src/can/canConfig.cpp            
            src/spi/zephyrSpi.cpp 
            src/spi/spiFactory.cpp)

#add_subdirectory(src/can)

target_sources(${TARGET_NAME} PRIVATE ${sources})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
