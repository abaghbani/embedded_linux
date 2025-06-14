
GNU_VERSION := 14.2.1
# GNU_PREFIX := aarch64-none-linux-gnu
GNU_PREFIX := arm-none-linux-gnueabihf

OUTPUT_DIRECTORY := ./_build
TARGETS := output.elf

LIB_DIR := ../../../lib
SRC_DIRS := ..
SRC_DIRS += $(LIB_DIR)/bcm2835
SRC_DIRS += $(LIB_DIR)/st7789/src

INC_FOLDERS := $(LIB_DIR)/bcm2835
INC_FOLDERS += $(LIB_DIR)/st7789/include

SRC_C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
SRC_CPP_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
SRC_FILES := $(SRC_C_FILES) $(SRC_CPP_FILES)

OBJ_C_FILES := $(addprefix $(OUTPUT_DIRECTORY)/, $(notdir $(SRC_C_FILES:.c=.o)))
OBJ_CPP_FILES := $(addprefix $(OUTPUT_DIRECTORY)/, $(notdir $(SRC_CPP_FILES:.cpp=.o)))
OBJ_FILES := $(OBJ_C_FILES) $(OBJ_CPP_FILES)

vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)

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
CFLAGS += $(foreach dir, $(INC_FOLDERS), -I$(dir))
CFLAGS += -c
CFLAGS += -pthread
CFLAGS += -fmessage-length=0
CFLAGS += -MT"$@"
CFLAGS += -MMD -MP -MF"$(@:.o=.d)"

CXXFLAGS += -Wall
CXXFLAGS += -O0
CXXFLAGS += -g3
CXXFLAGS += $(foreach dir, $(INC_FOLDERS), -I$(dir))
CXXFLAGS += -c
CXXFLAGS += -pthread
CXXFLAGS += -fmessage-length=0
CXXFLAGS += -std=c++17
CXXFLAGS += -MT"$@"
CXXFLAGS += -MMD -MP -MF"$(@:.o=.d)"

# Linker flags
# LDFLAGS += -O3 -g3
# LDFLAGS += -mthumb -mabi=aapcs -T$(LINKER_SCRIPT)
# LDFLAGS += -L "M:/home/akbar/sdr_linux/build/tmp/work"
LDFLAGS += -pthread
LDFLAGS += -static

all: $(OUTPUT_DIRECTORY) $(TARGETS)

$(OUTPUT_DIRECTORY):
	@if [ ! -d "$@" ]; then mkdir -p "$@"; fi
	
$(TARGETS): $(OBJ_FILES)
	@echo Building target: $@
	@echo Invoking: ARM v7 Linux gcc linker
	@$(CXX) -o $@ $^ $(LDFLAGS)
	@echo Finished building target: $@
	@$(SIZE) $@

$(OUTPUT_DIRECTORY)/%.o: %.c
	@echo 'Compiling file: $<'
	@$(CC) $(CFLAGS) -o $@ $<

$(OUTPUT_DIRECTORY)/%.o: %.cpp
	@echo 'Compiling file: $<'
	@$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: clean, update_output, size

clean:
	@echo "Removing output folder ..."
	@rm -rf $(OUTPUT_DIRECTORY) $(TARGETS)

update_output:
	cp $(TARGETS) ../$(TARGETS)

size: $(TARGETS)
	@echo Invoking: ARM v7 Linux Print Size
	@$(SIZE) $<
