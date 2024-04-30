#!/bin/bash

set -euo pipefail

[[ $# -gt 0 ]] || {
    echo >&2 "usage: $0 PATH"
    echo >&2 "missing required PATH to copy artifacts into once they're built"
    echo >&2 "e.g. $0 ../learn-gpgpu/wasm"
    exit 2
}

set -x

cd "$(dirname "${BASH_SOURCE[0]}")/.."

docker build -f Dockerfile.emscripten . --target=talvos

docker run -it --rm  -v "$(pwd)":/usr/src/talvos -w /usr/src/talvos \
    "$(docker build -qf Dockerfile.emscripten . --target=talvos)" -- \
    cmake --build build/emscripten-docker --target talvos-wasm

cp --verbose --update=older build/emscripten-docker/tools/talvos-cmd/talvos-wasm.* "$1"
