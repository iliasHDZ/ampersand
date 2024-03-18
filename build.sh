#!/bin/bash

set -o xtrace

source ./params.sh

PREV_PATH=$(pwd)
cd "${BUILD_PATH}"

BUILD_SUCCESS=1

make

if [ $? -eq 0 ]; then
    BUILD_SUCCESS=1
else
    BUILD_SUCCESS=0
fi

cd "${PREV_PATH}"

cp "${BUILD_PATH}/kernel" "./iso/boot/kernel.bin"

if [ ${BUILD_SUCCESS} -eq 1 ]; then
    grub-mkrescue /usr/lib/grub/i386-pc -o "${BUILD_PATH}/ampersand.iso" ./iso
fi