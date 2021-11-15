set(JERRY_ROOT ${CMAKE_SOURCE_DIR}/lib/jerryscript)
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
  --profile=es.next #es2015-subset
  --jerry-cmdline=OFF
  --cpointer-32bit=ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(KALUMA_GENERATED_C
  ${SRC_DIR}/gen/kaluma_modules.c
  ${SRC_DIR}/gen/kaluma_magic_strings.c)
set(KALUMA_GENERATED_H
  ${SRC_DIR}/gen/kaluma_modules.h
  ${SRC_DIR}/gen/kaluma_magic_strings.h)
file(GLOB_RECURSE KALUMA_MODULE_SRC ${SRC_DIR}/modules/*)
set(KALUMA_GENERATED ${KALUMA_GENERATED_C} ${KALUMA_GENERATED_H})

string (REPLACE ";" " " KALUMA_MODULE_LIST "${KALUMA_MODULES}")

set(JERRY_LIBS
  ${JERRY_ROOT}/build/lib/libjerry-core.a
  ${JERRY_ROOT}/build/lib/libjerry-ext.a)

add_custom_command(OUTPUT ${JERRY_LIBS}
  DEPENDS ${KALUMA_GENERATED_C} ${KALUMA_MODULE_SRC}
  WORKING_DIRECTORY ${JERRY_ROOT}
  COMMAND python tools/build.py --clean ${JERRY_ARGS})

add_custom_command(OUTPUT ${KALUMA_GENERATED_C}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND python ${JERRY_ROOT}/tools/build.py --clean --jerry-cmdline-snapshot=ON --snapshot-save=ON --snapshot-exec=ON --profile=es.next #es2015-subset
  COMMAND node tools/js2c.js --modules=${KALUMA_MODULE_LIST} --target=${TARGET}
  COMMAND rm -rf lib/jerryscript/build)

set(KALUMA_INC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/port ${SRC_DIR}/gen ${SRC_DIR}/modules)
include_directories(${KALUMA_INC} ${JERRY_INC})

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
  ${KALUMA_GENERATED_C})

# KALUMA MODULES -------------------------------------------------------------

if("pwm" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES ${SRC_DIR}/modules/pwm/module_pwm.c)
  include_directories(${SRC_DIR}/modules/pwm)
endif()

if("i2c" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES ${SRC_DIR}/modules/i2c/module_i2c.c)
  include_directories(${SRC_DIR}/modules/i2c)
endif()

if("spi" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES ${SRC_DIR}/modules/spi/module_spi.c)
  include_directories(${SRC_DIR}/modules/spi)
endif()

if("storage" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES ${SRC_DIR}/modules/storage/module_storage.c)
  include_directories(${SRC_DIR}/modules/storage)
endif()

if("uart" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES ${SRC_DIR}/modules/uart/module_uart.c)
  include_directories(${SRC_DIR}/modules/uart)
endif()

if("graphics" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES
    ${SRC_DIR}/modules/graphics/gc_cb_prims.c
    ${SRC_DIR}/modules/graphics/gc_1bit_prims.c
    ${SRC_DIR}/modules/graphics/gc_16bit_prims.c
    ${SRC_DIR}/modules/graphics/gc.c
    ${SRC_DIR}/modules/graphics/font_default.c
    ${SRC_DIR}/modules/graphics/module_graphics.c)
  include_directories(${SRC_DIR}/modules/graphics)
endif()

if("rp2" IN_LIST KALUMA_MODULES)
  list(APPEND SOURCES ${SRC_DIR}/modules/rp2/module_rp2.c)
  include_directories(${SRC_DIR}/modules/rp2)
endif()
