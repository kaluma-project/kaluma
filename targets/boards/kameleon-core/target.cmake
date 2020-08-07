add_definitions(-DUSE_HAL_DRIVER
  -DSTM32F411xE 
  -DUSE_FULL_ASSERT)

set(TARGET_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(TARGET_SHARED_DIR ${CMAKE_CURRENT_LIST_DIR}/../../shared)

set(SOURCES
  ${SOURCES}
  ${TARGET_SRC_DIR}/startup_stm32f411xe.s
  ${TARGET_SRC_DIR}/adc.c
  ${TARGET_SRC_DIR}/ringbuffer.c
  ${TARGET_SRC_DIR}/system.c
  ${TARGET_SRC_DIR}/gpio.c
  ${TARGET_SRC_DIR}/pwm.c
  ${TARGET_SRC_DIR}/tty.c
  ${TARGET_SRC_DIR}/usb_device.c
  ${TARGET_SRC_DIR}/usbd_conf.c
  ${TARGET_SRC_DIR}/stm32f4xx_it.c
  ${TARGET_SRC_DIR}/usbd_desc.c
  ${TARGET_SRC_DIR}/usbd_cdc_if.c
  ${TARGET_SRC_DIR}/system_stm32f4xx.c
  ${TARGET_SRC_DIR}/stm32f4xx_hal_msp.c
  ${TARGET_SRC_DIR}/flash.c
  ${TARGET_SRC_DIR}/storage.c
  ${TARGET_SRC_DIR}/uart.c
  ${TARGET_SRC_DIR}/i2c.c
  ${TARGET_SRC_DIR}/spi.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
  ${TARGET_SHARED_DIR}/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
  ${TARGET_SHARED_DIR}/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
  ${TARGET_SHARED_DIR}/middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
  ${TARGET_SHARED_DIR}/middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc)
  
include_directories(${CMAKE_CURRENT_LIST_DIR}/include
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Inc
  ${TARGET_SHARED_DIR}/drivers/STM32F4xx_HAL_Driver/Inc/Legacy
  ${TARGET_SHARED_DIR}/middlewares/ST/STM32_USB_Device_Library/Core/Inc
  ${TARGET_SHARED_DIR}/middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
  ${TARGET_SHARED_DIR}/drivers/CMSIS/Device/ST/STM32F4xx/Include
  ${TARGET_SHARED_DIR}/drivers/CMSIS/Include)

set(TARGET_HEAPSIZE 96)

if (BOOTLOADER)
  set(TARGET_LDSCRIPT ${TARGET_SRC_DIR}/STM32F411CETx_FLASH_BOOT.ld)
else()
  set(TARGET_LDSCRIPT ${TARGET_SRC_DIR}/STM32F411CETx_FLASH.ld)
endif()

set(KAMELEON_MODULES events gpio led button pwm adc i2c spi uart graphics at storage stream net http url wifi startup)

set(KAMELEON_MODULE_PWM 1)
set(KAMELEON_MODULE_I2C 1)
set(KAMELEON_MODULE_SPI 1)
set(KAMELEON_MODULE_STORAGE 1)
set(KAMELEON_MODULE_UART 1)
set(KAMELEON_MODULE_GRAPHICS 1)

set(CMAKE_EXE_LINKER_FLAGS "-specs=nano.specs -u _printf_float -T${TARGET_LDSCRIPT} -Wl,-Map=kameleon-core.map,--cref -Wl,--gc-sections")
