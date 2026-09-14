# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(HvacWaterValve)

set(TARGET app)
add_subdirectory(drivers)
include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/drivers/include>)
                    
target_sources(${TARGET} PRIVATE
  src/main.cpp
  src/drv8838.cpp
  )
target_link_libraries(${TARGET} PRIVATE drivers)