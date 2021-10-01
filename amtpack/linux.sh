#!/bin/sh

if [[ ! -d "../build" ]];
then
    mkdir "../build"
fi

pushd "../build" > /dev/null

COMPILER_FLAGS="-Wall -Wno-unused-function -I ../base"
LINKER_FLAGS="-ldl -lpthread"

# Build
#
g++ -O0 -g -ggdb $COMPILER_FLAGS "../amtpack/linux_amtpack.cpp" -o "amtpack" $LINKER_FLAGS

popd > /dev/null
