#!/bin/bash

TARGET_HEAPSIZE=96

JERRY_ARGS="\
--toolchain=../../esp32/components/jerryscript/toolchain_mcu_esp32.cmake \
--lto=OFF \
--error-messages=ON \
--js-parser=ON \
--mem-heap=${TARGET_HEAPSIZE} \
--mem-stats=ON \
--snapshot-exec=ON \
--line-info=ON \
--vm-exec-stop=ON \
--profile=es2015-subset \
--jerry-cmdline=OFF \
--cpointer-32bit=ON"

../../../deps/jerryscript/tools/build.py --clean ${JERRY_ARGS}

cp ../../../deps/jerryscript/build/lib/*.a .
