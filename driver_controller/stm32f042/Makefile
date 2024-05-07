export CROSS_COMPILE ?= arm-none-eabi-
export CC := $(CROSS_COMPILE)gcc	
export CPP := $(CROSS_COMPILE)g++	
export CPU ?= cortex-m0
export ARCH ?= armv6-m
export APPDIR := application
export TOOL ?= gnu
export OBJCOPY := $(CROSS_COMPILE)objcopy
export LIB_PATH := lib
export LINKER_FILE := gcc_arm.ld


PROJECT_NAME := driver_controller

export AR := $(CROSS_COMPILE)ar
export CFLAGS := -mcpu=$(CPU) -march=$(ARCH) -mthumb -O0 \
		   -fmessage-length=0 -fsigned-char -ffunction-sections \
		   -fdata-sections -g3 -DSTM32F042x6 -D__NO_SYSTEM_INIT -D__STARTUP_CLEAR_BSS_MULTIPLE

LDFLAGS = -mcpu=$(CPU)  \
		--specs=nosys.specs -Wl,-Map="$(PROJECT_NAME).map" -Wl,--gc-sections -static \
		--specs=nano.specs

SHELL:=/bin/bash

export LIBS_DIR =  $(LIB_PATH)/STM32F0xx_HAL_Driver/

BUILD_BUILTIN := 1

export BUILD_BUILTIN
export PROGDIR = $(CURDIR)
export BUILDDIR = $(CURDIR)/build
export LIBDIR ?= $(CURDIR)/build
export OBJDIR ?= $(LIBDIR)/obj

srctree := .
objtree := .
VPATH := $(src_tree)

export srctree objtree VPATH

MAKEFLAGS += -rR --no-print-directory

include ./scripts/Makefile.include

ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif

ifeq ($(VERBOSE),1)
  Q =
else
  Q = @
endif
export Q
export VERBOSE
export INCLUDES

#ifeq ($(findstring liblpc,$(MAKECMDGOALS)),liblpc)
#include sources.mk
#VPATH += $(CURDIR)/$(LPC_DIR)/src
#$(shell mkdir -p $(OBJDIR)/$(LIBS_DIR))
#endif

export cmd_file := application/$(LINKER_FILE)
_dirs := Core/ $(LIBS_DIR) application/ 

built-ins := $(patsubst %/,%/built-in.a, $(_dirs))

PHONY = $(built-ins) FORCE

$(PROJECT_NAME).hex : $(PROJECT_NAME).elf
	$(Q)$(OBJCOPY) -O ihex $< $@ 

$(PROJECT_NAME).elf : $(built-ins) FORCE
	$(Q) echo "Linking Application:  $@"
	$(Q)$(CPP) -Wl,--start-group $(built-ins) -Wl,--end-group  -T $(cmd_file) $(LDFLAGS) -o $@ 

$(built-ins): FORCE
	$(Q)$(MAKE) $(build)=$(patsubst %/,%,$(dir $@)) \
	need-builtin=1

FORCE:

clean: 
	@rm -f *.map *.elf
	@find -name '*.[aod]' -type f -print | xargs rm -f

clean_app:
	@rm -f *.map *.elf
	@find ./application -name '*.[aod]' -type f -print  | xargs rm -f

.phony = $(PHONY)
