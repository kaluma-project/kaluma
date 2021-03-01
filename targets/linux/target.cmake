######################################
# building variables
######################################
# debug build?
set(DEBUG 1)
# optimization
set(OPT -Og)

project(kaluma-project C)
set(TARGET_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(TARGET_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(SOURCES
  ${SOURCES}
  ${TARGET_SRC_DIR}/adc.c
  ${TARGET_SRC_DIR}/ringbuffer.c
  ${TARGET_SRC_DIR}/system.c
  ${TARGET_SRC_DIR}/gpio.c
  ${TARGET_SRC_DIR}/pwm.c
  ${TARGET_SRC_DIR}/tty.c
  ${TARGET_SRC_DIR}/flash.c
  ${TARGET_SRC_DIR}/storage.c
  ${TARGET_SRC_DIR}/uart.c
  ${TARGET_SRC_DIR}/i2c.c
  ${TARGET_SRC_DIR}/spi.c)

include_directories(${TARGET_INC_DIR})

set(TARGET_HEAPSIZE 96)
set(JERRY_TOOLCHAIN toolchain_linux_i686.cmake)

set(KALUMA_MODULES events gpio led button pwm adc i2c spi uart graphics at storage stream http url startup)

set(CMAKE_SYSTEM_PROCESSOR amd64)
set(CMAKE_C_FLAGS "${OPT} -Wall -fdata-sections -ffunction-sections")
if(DEBUG EQUAL 1)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -gdwarf-2")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -MMD -MP")

#set(PREFIX)
set(CMAKE_C_COMPILER ${PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${PREFIX}g++)
set(CMAKE_LINKER ${PREFIX}ld)
set(CMAKE_OBJCOPY ${PREFIX}objcopy)

set(TARGET_LIBS c m)
set(CMAKE_EXE_LINKER_FLAGS "-u _printf_float -Wl,-Map=linux.map,--cref -Wl,--gc-sections")