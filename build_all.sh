#!/bin/bash

set -euxo pipefail

./build_debug.sh

./build_release.sh

