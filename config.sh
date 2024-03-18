#!/bin/bash

set -o xtrace

source ./params.sh

cmake -G "Unix Makefiles" \
      -DCMAKE_C_COMPILER="${C_COMPILER_PATH}" \
      -DCMAKE_CXX_COMPILER="${CXX_COMPILER_PATH}" \
      -DCMAKE_CXX_COMPILER_WORKS=1 \
      -DCMAKE_C_COMPILER_WORKS=1 \
      -DTARGET_ARCH="${TARGET_ARCH}" \
      -B "${BUILD_PATH}" \
      -S .