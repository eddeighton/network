#!/bin/bash

set -euxo pipefail


cmake -S . -B /build/debug \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_FLAGS=-fdiagnostics-urls=always \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=20 \
    -GNinja \
    -Wno-dev

