#!/bin/bash

pushd ~/Work/kameleon/esp32/main/targets/boards/esp32-devkitc/js
make
popd

. ~/Work/esp-idf/export.sh
./build_modules.sh
idf.py build
