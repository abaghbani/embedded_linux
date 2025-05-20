
GNU_VERSION := 14.2.1
GNU_PREFIX := aarch64-none-linux-gnu

OUTPUT_DIRECTORY := ./_build
TARGETS := output.elf

LIB_DIR := ../../../lib
SRC_DIRS := ..
SRC_DIRS += $(LIB_DIR)/i2c

INC_FOLDERS := $(LIB_DIR)/i2c/

SRC_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJ_FILES := $(addprefix $(OUTPUT_DIRECTORY)/, $(notdir $(SRC_FILES:.c=.o)))
vpath %.c $(SRC_DIRS)

# Toolchain commands
CC      := $(GNU_PREFIX)-gcc-$(GNU_VERSION)
CXX     := $(GNU_PREFIX)-c++
AS      := $(GNU_PREFIX)-as
AR      := $(GNU_PREFIX)-ar -r
LD      := $(GNU_PREFIX)-ld
NM      := $(GNU_PREFIX)-nm
OBJDUMP := $(GNU_PREFIX)-objdump
OBJCOPY := $(GNU_PREFIX)-objcopy
SIZE    := $(GNU_PREFIX)-size

# C flags
CFLAGS += -Wall
CFLAGS += -O0
CFLAGS += -g3
CFLAGS += -I "$(INC_FOLDERS)"
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
	@if [ ! -d "$@" ]; then mkdir -p "$@"; fi
	
$(TARGETS): $(OBJ_FILES)
	@echo Building target: $@
	@echo Invoking: ARM v7 Linux gcc linker
	@$(CC) -o $@ $^ $(LDFLAGS)
	@echo Finished building target: $@
	@$(SIZE) $@

$(OUTPUT_DIRECTORY)/%.o: %.c
	@echo 'Compiling file: $<'
	@$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean, update_output, size

clean:
	rm -rf $(OUTPUT_DIRECTORY) $(TARGETS)

update_output:
	cp $(TARGETS) ../$(TARGETS)

size: $(TARGETS)
	@echo Invoking: ARM v7 Linux Print Size
	@$(SIZE) $<