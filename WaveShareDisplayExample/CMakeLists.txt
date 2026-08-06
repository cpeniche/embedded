# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Pull in our own Kconfig (which sources Kconfig.zephyr plus the
# out-of-tree driver options under drivers/) instead of Zephyr's
# default root Kconfig.
set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(display)
set(TARGET_NAME app)
set(sources src/main.c
#[[
    User/Config/DEV_Config.c
    User/e-Paper/EPD_2in15g.c
    User/Examples/EPD_2in15g_test.c
    User/Examples/ImageData.c
    User/Fonts/font8.c
    User/Fonts/font12.c
    User/Fonts/font16.c
    User/Fonts/font20.c
    User/Fonts/font24.c
    User/Fonts/font24CN.c
    User/GUI/GUI_Paint.c#]])

if(CONFIG_JD79661)
  list(APPEND sources drivers/jd79661.c)
endif()

target_sources(${TARGET_NAME} PRIVATE ${sources})
#[[
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/User/e-Paper 
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/User/Examples
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/User/Fonts
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/User/GUI
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/User/Config
                                                 )#]]
