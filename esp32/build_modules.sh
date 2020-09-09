#!/bin/bash

cd ..

python deps/jerryscript/tools/build.py --clean --jerry-cmdline-snapshot=ON --snapshot-save=ON --snapshot-exec=ON --profile=es2015-subset
node tools/js2c.js --modules='events gpio led button pwm adc i2c spi uart graphics at storage stream net http url wifi startup' --target=esp32-devkitc
rm -rf deps/jerryscript/build

cd esp32/components/jerryscript

./build.sh

