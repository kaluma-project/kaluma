# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
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
# target
######################################
TARGET = kameleon-core


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

# Target path
TARGET_DIR = targets/$(TARGET)

######################################
# source
######################################
# C sources
CSRC =  \
src/main.c \
src/kameleon_list.c \
src/kameleon_io.c \
$(TARGET_DIR)/src/kameleon_system.c \
$(TARGET_DIR)/src/kameleon_gpio.c \
$(TARGET_DIR)/src/kameleon_tty.c \
$(TARGET_DIR)/src/usb_device.c \
$(TARGET_DIR)/src/usbd_conf.c \
$(TARGET_DIR)/src/stm32f4xx_it.c \
$(TARGET_DIR)/src/usbd_desc.c \
$(TARGET_DIR)/src/usbd_cdc_if.c \
$(TARGET_DIR)/src/system_stm32f4xx.c \
$(TARGET_DIR)/src/stm32f4xx_hal_msp.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c \
$(TARGET_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
$(TARGET_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
$(TARGET_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
$(TARGET_DIR)/middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c

# ASM sources
ASRC = \
$(TARGET_DIR)/src/startup_stm32f411xe.s


######################################
# firmware library
######################################
PERIFLIB_SOURCES =


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F411xE

# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
-Iinclude \
-Iinclude/port \
-I$(TARGET_DIR)/include \
-I$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Inc \
-I$(TARGET_DIR)/drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
-I$(TARGET_DIR)/middlewares/ST/STM32_USB_Device_Library/Core/Inc \
-I$(TARGET_DIR)/middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
-I$(TARGET_DIR)/drivers/CMSIS/Device/ST/STM32F4xx/Include \
-I$(TARGET_DIR)/drivers/CMSIS/Include


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

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
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(CSRC:.c=.o)))
vpath %.c $(sort $(dir $(CSRC)))
# list of ASM program objects
OBJS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASRC:.s=.o)))
vpath %.s $(sort $(dir $(ASRC)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "compile:" $<
	$(Q) $(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "compile:" $<
	$(Q) $(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) Makefile
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

#######################################
# clean up
#######################################
clean:
	$(Q) -rm -fR .dep $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
