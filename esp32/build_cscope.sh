#!/bin/bash

find main -name "*.c" > cscope.files
find ../src -name "*.c" >> cscope.files
find ../deps -name "*.c" >> cscope.files
find ../deps -name "*.cpp" >> cscope.files
find main -name "*.h" >> cscope.files
find ../src -name "*.h" >> cscope.files
find ../deps -name "*.h" >> cscope.files

for DIR in newlib vfs driver esp32 
do
    find $IDF_PATH/components/$DIR -name "*.c" >> cscope.files
    find $IDF_PATH/components/$DIR -name "*.h" >> cscope.files
done

cscope -b
