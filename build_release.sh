#!/bin/bash

set -euxo pipefail

# cmake -S . -B /build/release \
#     -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_CXX_STANDARD=20 \
#     -GNinja \
#     -Wno-dev
cmake -S . -B /build/release \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    -GNinja \
    -Wno-dev

