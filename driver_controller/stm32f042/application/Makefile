INCLUDES = -Iapplication/include
INCLUDES += -Ilib/STM32F0xx_HAL_Driver/include 
INCLUDES += -Ilib/CMSIS/Device/ST/STM32F0xx/include
INCLUDES += -Ilib/CMSIS/include
INCLUDES += -ICore/include
INCLUDES += -Iapplication/driver_model/include
INCLUDES += -Ilib/FreeRTOSv202210.01-LTS/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/include
INCLUDES += -Ilib/FreeRTOSv202210.01-LTS/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM0

#CFLAGS += -Wa,-adhln -g

obj-y = src/ driver_model/src/

export cmd_file := application/gcc_arm.ld