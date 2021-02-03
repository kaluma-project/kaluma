######################################
# building variables
######################################
# debug build?
set(DEBUG 1)
# optimization
set(OPT -Og)

include(${CMAKE_SOURCE_DIR}/targets/boards/${TARGET}/target.cmake)

set(JERRY_ROOT ${CMAKE_SOURCE_DIR}/deps/jerryscript)
set(JERRY_INC
  ${JERRY_ROOT}/jerry-core
  ${JERRY_ROOT}/jerry-core/api
  ${JERRY_ROOT}/jerry-core/debugger
  ${JERRY_ROOT}/jerry-core/ecma/base
  ${JERRY_ROOT}/jerry-core/ecma/builtin-objects
  ${JERRY_ROOT}/jerry-core/ecma/builtin-objects/typedarray
  ${JERRY_ROOT}/jerry-core/ecma/operations
  ${JERRY_ROOT}/jerry-core/include
  ${JERRY_ROOT}/jerry-core/jcontext
  ${JERRY_ROOT}/jerry-core/jmem
  ${JERRY_ROOT}/jerry-core/jrt
  ${JERRY_ROOT}/jerry-core/lit
  ${JERRY_ROOT}/jerry-core/parser/js
  ${JERRY_ROOT}/jerry-core/parser/regexp
  ${JERRY_ROOT}/jerry-core/vm
  ${JERRY_ROOT}/jerry-ext/arg
  ${JERRY_ROOT}/jerry-ext/include
  ${JERRY_ROOT}/jerry-libm)

set(JERRY_ARGS
  --toolchain=cmake/${JERRY_TOOLCHAIN}
  --lto=OFF
  --error-messages=ON
  --js-parser=ON
  --mem-heap=${TARGET_HEAPSIZE}
  --mem-stats=ON
  --snapshot-exec=ON
  --line-info=ON
  --vm-exec-stop=ON
  --profile=es2015-subset
  --jerry-cmdline=OFF
  --cpointer-32bit=ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(KAMELEON_GENERATED_C
  ${SRC_DIR}/gen/kameleon_modules.c
  ${SRC_DIR}/gen/kameleon_magic_strings.c)
set(KAMELEON_GENERATED_H
  ${SRC_DIR}/gen/kameleon_modules.h
  ${SRC_DIR}/gen/kameleon_magic_strings.h)

set(KAMELEON_GENERATED ${KAMELEON_GENERATED_C} ${KAMELEON_GENERATED_H})

string (REPLACE ";" " " KAMELEON_MODULE_LIST "${KAMELEON_MODULES}")

set(JERRY_LIBS
  ${JERRY_ROOT}/build/lib/libjerry-core.a
  ${JERRY_ROOT}/build/lib/libjerry-ext.a)

add_custom_command(OUTPUT ${JERRY_LIBS}
  DEPENDS ${KAMELEON_GENERATED_C}
  WORKING_DIRECTORY ${JERRY_ROOT}
  COMMAND python tools/build.py --clean ${JERRY_ARGS})

add_custom_command(OUTPUT ${KAMELEON_GENERATED_C}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND python ${JERRY_ROOT}/tools/build.py --clean --jerry-cmdline-snapshot=ON --snapshot-save=ON --snapshot-exec=ON --profile=es2015-subset
  COMMAND node tools/js2c.js --modules=${KAMELEON_MODULE_LIST} --target=${TARGET}
  COMMAND rm -rf deps/jerryscript/build)

# Delete generaged file to generate it every time.
add_custom_command(OUTPUT clean_gen
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND rm -f src/gen/*)
#=============================================================
set(KAMELEON_INC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/port ${SRC_DIR}/gen ${SRC_DIR}/modules)
include_directories(${KAMELEON_INC} ${JERRY_INC})

list(APPEND SOURCES
  ${SRC_DIR}/main.c
  ${SRC_DIR}/utils.c
  ${SRC_DIR}/base64.c
  ${SRC_DIR}/io.c
  ${SRC_DIR}/runtime.c
  ${SRC_DIR}/repl.c
  ${SRC_DIR}/jerry_port.c
  ${SRC_DIR}/jerryxx.c
  ${SRC_DIR}/global.c
  ${SRC_DIR}/ymodem.c
  ${KAMELEON_GENERATED_C})

if(KAMELEON_MODULE_PWM)
  list(APPEND SOURCES ${SRC_DIR}/modules/pwm/module_pwm.c)
  include_directories(${SRC_DIR}/modules/pwm)
endif()
if(KAMELEON_MODULE_I2C)
  list(APPEND SOURCES ${SRC_DIR}/modules/i2c/module_i2c.c)
  include_directories(${SRC_DIR}/modules/i2c)
endif()
if(KAMELEON_MODULE_SPI)
  list(APPEND SOURCES ${SRC_DIR}/modules/spi/module_spi.c)
  include_directories(${SRC_DIR}/modules/spi)
endif()
if(KAMELEON_MODULE_STORAGE)
  list(APPEND SOURCES ${SRC_DIR}/modules/storage/module_storage.c)
  include_directories(${SRC_DIR}/modules/storage)  
endif()
if(KAMELEON_MODULE_UART)
  list(APPEND SOURCES ${SRC_DIR}/modules/uart/module_uart.c)
  include_directories(${SRC_DIR}/modules/uart)  
endif()
if(KAMELEON_MODULE_GRAPHICS)
  list(APPEND SOURCES
    ${SRC_DIR}/modules/graphics/gc_cb_prims.c
    ${SRC_DIR}/modules/graphics/gc_1bit_prims.c
    ${SRC_DIR}/modules/graphics/gc_16bit_prims.c
    ${SRC_DIR}/modules/graphics/gc.c
    ${SRC_DIR}/modules/graphics/font_default.c
    ${SRC_DIR}/modules/graphics/module_graphics.c)
  include_directories(${SRC_DIR}/modules/graphics)
endif()

add_executable(${TARGET}.elf ${SOURCES} ${JERRY_LIBS} clean_gen)
target_link_libraries(${TARGET}.elf ${JERRY_LIBS} ${TARGET_LIBS})

add_custom_command(OUTPUT ${TARGET}.hex ${TARGET}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET}.elf ${TARGET}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary -S ${TARGET}.elf ${TARGET}.bin
  DEPENDS ${TARGET}.elf)

add_custom_target(kameleon ALL DEPENDS ${TARGET}.hex ${TARGET}.bin)
