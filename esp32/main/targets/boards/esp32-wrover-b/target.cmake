set(TARGET_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/src/adc.c
  ${CMAKE_CURRENT_LIST_DIR}/src/flash.c
  ${CMAKE_CURRENT_LIST_DIR}/src/nvs.c
  ${CMAKE_CURRENT_LIST_DIR}/src/gpio.c
  ${CMAKE_CURRENT_LIST_DIR}/src/i2c.c
  ${CMAKE_CURRENT_LIST_DIR}/src/i2c_freertos.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pwm.c
  ${CMAKE_CURRENT_LIST_DIR}/src/spi.c
  ${CMAKE_CURRENT_LIST_DIR}/src/storage.c
  ${CMAKE_CURRENT_LIST_DIR}/src/system.c
  ${CMAKE_CURRENT_LIST_DIR}/src/tty.c
  ${CMAKE_CURRENT_LIST_DIR}/src/uart.c)

set(TARGET_INCLUDE
  ${CMAKE_CURRENT_LIST_DIR}/include)

