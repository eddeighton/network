#!/bin/bash

set -euxo pipefail

cmake -S . -B /build/asan \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=20 \
    -GNinja \
    -Wno-dev \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"

