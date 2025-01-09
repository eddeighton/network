#!/bin/bash

set -euxo pipefail

cmake -S . -B /build/debug \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=20 \
    -DBOOST_STACKTRACE_USE_ADDR2LINE=1 \
    -GNinja \
    -Wno-dev

