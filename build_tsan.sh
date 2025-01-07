#!/bin/bash

set -euxo pipefail

cmake -S . -B /build/tsan \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=20 \
    -GNinja \
    -Wno-dev \
    -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer -fpic -fpie -fPIE" \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libtsan"

