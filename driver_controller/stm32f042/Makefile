export CROSS_COMPILE ?= arm-none-eabi-
export CC := $(CROSS_COMPILE)gcc	
export CPP := $(CROSS_COMPILE)g++	
export CPU ?= cortex-m0
export ARCH ?= armv6-m
export APPDIR := application
export TOOL ?= gnu
export OBJCOPY := $(CROSS_COMPILE)objcopy
export SIZE := $(CROSS_COMPILE)size
export LIB_PATH := lib
export LINKER_FILE := gcc_arm.ld


PROJECT_NAME := driver_controller

export AR := $(CROSS_COMPILE)ar
export CFLAGS := -mcpu=$(CPU) -march=$(ARCH) -mthumb -O0 -specs=nosys.specs  \
		   -fmessage-length=0 -fsigned-char -ffunction-sections --specs=nano.specs \
		   -fdata-sections -g3 -DSTM32F042x6 -D__NO_SYSTEM_INIT -D__STARTUP_CLEAR_BSS_MULTIPLE

#LDFLAGS = -mcpu=$(CPU)  \

#		--specs=nosys.specs \   
LDFLAGS=-Wl,-Map="$(PROJECT_NAME).map" \
		-Wl,--gc-sections \
		-Wl,--static
#		--specs=nano.specs    


SHELL:=/bin/bash

export LIBS_DIR =  $(LIB_PATH)/STM32F0xx_HAL_Driver/ $(LIB_PATH)/FreeRTOSv202210.01-LTS/

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

export cmd_file := application/$(LINKER_FILE)
_dirs := $(LIBS_DIR) application/
_dirs_whole_archive := Core/ 

built-ins := $(patsubst %/,%/built-in.a, $(_dirs))
#Needed when the archive needs to override weak definitions on other 
#files that do not   
built-ins_whole_archive := $(patsubst %/,%/built-in.a, $(_dirs_whole_archive))

PHONY = $(built-ins_whole_archive) $(built-ins) FORCE



$(PROJECT_NAME).hex : $(PROJECT_NAME).elf
	$(Q)$(OBJCOPY) -O ihex $< $@;
	$(Q)$(SIZE) $(PROJECT_NAME).elf

$(PROJECT_NAME).elf : $(built-ins_whole_archive) $(built-ins) FORCE
	$(Q) echo "Linking Application:  $@"
	$(Q)$(CPP) -Wl,--start-group -Wl,--whole-archive $(built-ins_whole_archive) \
	           -Wl,--no-whole-archive $(built-ins) -Wl,--end-group  -T $(cmd_file) $(LDFLAGS) -o $@ 

$(built-ins): FORCE
	$(Q)$(MAKE) $(build)=$(patsubst %/,%,$(dir $@)) \
	need-builtin=1
	
$(built-ins_whole_archive): FORCE
	$(Q)$(MAKE) $(build)=$(patsubst %/,%,$(dir $@)) \
	need-builtin=1

FORCE:

clean: 
	@rm -f *.map *.elf *.hex
	@find -name '*.[aod]' -type f -print | xargs rm -f

clean_app:
	@rm -f *.map *.elf
	@find ./application -name '*.[aod]' -type f -print  | xargs rm -f

.phony = $(PHONY)
