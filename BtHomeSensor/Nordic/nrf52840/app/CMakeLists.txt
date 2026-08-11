# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(BtHome)

set(TARGET app)
add_subdirectory(drivers)
add_subdirectory(library/bmp280)
add_subdirectory(library/aht20)
include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/library/bmp280/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/library/aht20/include>
                    #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/library/waveshare/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/drivers/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/images>)
target_sources(${TARGET} PRIVATE
  src/main.cpp
  src/interface.c
  src/display.cpp
  images/dashboard.c
  )
#target_link_directories(${TARGET} PRIVATE ~/project/build/library/bmp280)
target_link_libraries(${TARGET} PRIVATE drivers bmp280 aht20)

