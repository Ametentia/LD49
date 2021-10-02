#!/bin/sh

if [[ ! -d "../build" ]];
then
    mkdir "../build"
fi

pushd "../build" > /dev/null

COMPILER_FLAGS="-Wall -Wno-unused-function -Wno-switch -I ../base"
LINKER_FLAGS="-ldl -lpthread"

# Build renderer
#
"../base/renderer/linux_glx.sh" debug

# Debug build
#
g++ -O0 -g -ggdb $COMPILER_FLAGS "../code/linux_ludum.cpp" -o "ludum.bin" $LINKER_FLAGS

# Release build
#
# g++ -O2 -fno-strict-aliasing -Werror $COMPILER_FLAGS "../code/linux_ludum.cpp" -o "ludum.bin" -link $LINKER_FLAGS

popd > /dev/null
