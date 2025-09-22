# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(driverModule)
set(TARGET_NAME app)
set(sources src/main.c 
            src/can/canConfig.c 
            src/spi/spiConfig.c)

#add_subdirectory(src/can)

target_sources(${TARGET_NAME} PRIVATE ${sources})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
#target_link_libraries(${TARGET_NAME} PRIVATE canConfig)
