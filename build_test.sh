#!/bin/bash

BUILD_SUCCESS=0

source ./config_build.sh

if [ ${BUILD_SUCCESS} -eq 1 ]; then
    source ./test.sh
fi