
# GNU_INSTALL_ROOT := C:/Users/Akbar/Documents/Tools/gnu_toolchain/gcc-arm-10.3-2021.07-mingw-w64-i686-arm-none-linux-gnueabihf/bin
# GNU_VERSION := 10.3.1
# GNU_PREFIX := arm-none-linux-gnueabihf
GNU_INSTALL_ROOT := C:/Xilinx/SDK/2019.1/gnu/aarch32/nt/gcc-arm-linux-gnueabi/bin
GNU_VERSION := 8.2.0
GNU_PREFIX := arm-linux-gnueabihf

PROJ_DIR := ../sources
OUTPUT_DIRECTORY := ./_build
TARGETS := sdr.elf

SRC_FILES := $(wildcard $(PROJ_DIR)/*.c)
# adding manually source files
# SRC_FILES += $(PROJ_DIR)/main.c

OBJ_FILES := $(SRC_FILES:$(PROJ_DIR)/%.c=$(OUTPUT_DIRECTORY)/%.o)

# Toolchain commands
CC      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-gcc-$(GNU_VERSION)
CXX     := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-c++
AS      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-as
AR      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-ar -r
LD      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-ld
NM      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-nm
OBJDUMP := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-objdump
OBJCOPY := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-objcopy
SIZE    := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-size
MK      := mkdir
RM      := rm -rf
CP      := cp

# C flags
CFLAGS += -Wall
CFLAGS += -O0
CFLAGS += -g3
#CFLAGS += -I "$(SRC_FILES)/include"
CFLAGS += -c
CFLAGS += -pthread
CFLAGS += -fmessage-length=0 
CFLAGS += -MT"$@" 
CFLAGS += -MMD 
CFLAGS += -MP 
CFLAGS += -MF"$(@:%.o=%.d)" 
CFLAGS += -MT"$(@)" 

# Linker flags
# LDFLAGS += -O3 -g3
# LDFLAGS += -mthumb -mabi=aapcs -T$(LINKER_SCRIPT)
# LDFLAGS += -L "M:/home/akbar/sdr_linux/build/tmp/work"
LDFLAGS += -pthread


all: $(OUTPUT_DIRECTORY) $(TARGETS)

$(OUTPUT_DIRECTORY):
	@if not exist $@/ $(MK) $@

$(TARGETS): $(OBJ_FILES)
	@echo Building target: $@
	@echo Invoking: ARM v7 Linux gcc linker
	@$(CC) -o $@ $^ $(LDFLAGS)
	@echo Finished building target: $@
	@$(SIZE) $@

$(OUTPUT_DIRECTORY)/%.o: $(PROJ_DIR)/%.c
	@echo 'Compiling file: $<'
	@$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean, update_output, size

clean:
	$(RM) $(OUTPUT_DIRECTORY) $(TARGETS)

update_output:
	$(CP) $(TARGETS) ../$(TARGETS)

size: $(TARGETS)
	@echo Invoking: ARM v7 Linux Print Size
	@$(SIZE) $<