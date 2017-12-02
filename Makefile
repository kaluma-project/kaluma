# ------------------------------------------------
# Makefile (based on gcc)
# ------------------------------------------------

# Control build verbosity
#
#  V=1,2: Enable echo of commands
#  V=2:   Enable bug/verbose options in tools and scripts

ifeq ($(V),1)
export Q :=
else
ifeq ($(V),2)
export Q :=
else
export Q := @
endif
endif

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

#######################################
# paths
#######################################

# Build path
BUILD_DIR = build

# -----------------------------------------------------------------------------
# Toolchain
# -----------------------------------------------------------------------------

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S


# -----------------------------------------------------------------------------
# Jerryscript
# -----------------------------------------------------------------------------

JERRY_ROOT = deps/jerryscript

JERRY_HOST = $(JERRY_ROOT)/build/bin/jerry

JERRY_LIBDIR = $(JERRY_ROOT)/build/lib

JERRY_LIBS = \
$(JERRY_LIBDIR)/libjerry-core.a \
$(JERRY_LIBDIR)/libjerry-ext.a

JERRY_INC = \
-I${JERRY_ROOT}/jerry-core \
-I${JERRY_ROOT}/jerry-core/api \
-I${JERRY_ROOT}/jerry-core/debugger \
-I${JERRY_ROOT}/jerry-core/ecma/base \
-I${JERRY_ROOT}/jerry-core/ecma/builtin-objects \
-I${JERRY_ROOT}/jerry-core/ecma/builtin-objects/typedarray \
-I${JERRY_ROOT}/jerry-core/ecma/operations \
-I${JERRY_ROOT}/jerry-core/include \
-I${JERRY_ROOT}/jerry-core/jcontext \
-I${JERRY_ROOT}/jerry-core/jmem \
-I${JERRY_ROOT}/jerry-core/jrt \
-I${JERRY_ROOT}/jerry-core/lit \
-I${JERRY_ROOT}/jerry-core/parser/js \
-I${JERRY_ROOT}/jerry-core/parser/regexp \
-I${JERRY_ROOT}/jerry-core/vm \
-I${JERRY_ROOT}/jerry-ext/arg \
-I${JERRY_ROOT}/jerry-ext/include \
-I${JERRY_ROOT}/jerry-libm

JERRY_ARGS = \
--toolchain=cmake/toolchain_mcu_stm32f4.cmake \
--lto=OFF \
--error-messages=ON \
--js-parser=ON \
--mem-heap=78 \
--snapshot-exec=ON \
--jerry-cmdline=OFF

# -----------------------------------------------------------------------------
# Kameleon
# -----------------------------------------------------------------------------

KAMELEON_GENERATED_C = src/gen/kameleon_modules.c
KAMELEON_GENERATED_H = src/gen/kameleon_modules.h
KAMELEON_GENERATED = $(KAMELEON_GENERATED_C) $(KAMELEON_GENERATED_H)

KAMELEON_DEF =

KAMELEON_ASM = 

KAMELEON_SRC = \
src/main.c \
src/utils.c \
src/io.c \
src/runtime.c \
src/repl.c \
src/jerry_port.c \
src/global.c \
$(KAMELEON_GENERATED_C)

KAMELEON_INC = \
-Iinclude \
-Iinclude/port \
-Isrc/gen \
-Isrc/modules

# -----------------------------------------------------------------------------
# Target-specific
# -----------------------------------------------------------------------------

ifndef TARGET 
TARGET = kameleon-core
endif

TARGET_DIR = targets/$(TARGET)

TARGET_ASM =
TARGET_SRC =
TARGET_INC =
TARGET_DEF =

-include $(TARGET_DIR)/Make.def

# -----------------------------------------------------------------------------
# Kameleon Modules
# -----------------------------------------------------------------------------

ifdef KAMELEON_MODULE_EVENTS
endif

ifdef KAMELEON_MODULE_TIMERS
KAMELEON_SRC += src/modules/timers/module_timers.c
KAMELEON_INC += -Isrc/modules/timers
endif

# -----------------------------------------------------------------------------
# CFLAGS
# -----------------------------------------------------------------------------

# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mlittle-endian -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS = $(TARGET_DEF)

ASRC = $(KAMELEON_ASM) $(TARGET_ASM)
AINC =

CSRC = $(TARGET_SRC) $(KAMELEON_SRC)
CINC = $(TARGET_INC) $(KAMELEON_INC) $(JERRY_INC)

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AINC) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(CINC) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################

# link script
LDSCRIPT = \
$(TARGET_DIR)/src/STM32F411RETx_FLASH.ld

# libraries
LIBS = -ljerry-core -ljerry-ext -lc -lnosys -lm
LIBDIR = -L$(JERRY_LIBDIR)
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# -----------------------------------------------------------------------------
# Default action: build all
# -----------------------------------------------------------------------------

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	$(Q) ls -al $(BUILD_DIR)/$(TARGET).*
	@echo "Done."

# -----------------------------------------------------------------------------
# JS snapshot generation
# -----------------------------------------------------------------------------

$(KAMELEON_GENERATED):
	$(Q) python $(JERRY_ROOT)/tools/build.py --clean --jerry-cmdline-snapshot=ON --snapshot-save=ON --snapshot-exec=ON
	$(Q) node tools/js2c.js --modules=$(KAMELEON_MODULES)
	$(Q) -rm -rf deps/jerryscript/build

# -----------------------------------------------------------------------------
# Build app
# -----------------------------------------------------------------------------

# list of objects
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(CSRC:.c=.o)))
vpath %.c $(sort $(dir $(CSRC)))

# list of ASM program objects
OBJS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASRC:.s=.o)))
vpath %.s $(sort $(dir $(ASRC)))

$(BUILD_DIR)/%.o: %.c Makefile $(KAMELEON_GENERATED) | $(BUILD_DIR)
	@echo "compile:" $<
	$(Q) $(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "compile:" $<
	$(Q) $(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(JERRY_LIBS) Makefile
	@echo "link:" $@
	$(Q) $(CC) $(OBJS) $(LDFLAGS) -o $@
	$(Q) $(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "hex:" $@
	$(Q) $(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "bin:" $@
	$(Q) $(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

$(JERRY_LIBS):
	$(Q) python $(JERRY_ROOT)/tools/build.py --clean $(JERRY_ARGS)


clean:
	$(Q) -rm -rf deps/jerryscript/build
	$(Q) -rm $(KAMELEON_GENERATED)
	$(Q) -rm -fR $(BUILD_DIR)

flash:
	$(Q) st-flash write build/kameleon-core.bin 0x8000000

# *** EOF ***
