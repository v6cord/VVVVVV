#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

CC="${CC:-cc}"
if echo $'#ifdef _WIN32\nyes\n#endif' | $CC -E - | tail -n1 | grep -q yes; then
    windows=1
fi

if [ ! -d "build.$CC" ]; then
    rm -rf build
fi

mkdir -pv build.$CC/
ln -sfvn build.$CC build
cd build/

timestamp_ref="$(mktemp)"
function finish {
    rm -f "$timestamp_ref"
}
trap finish EXIT

if [ -e v6cord.png.c ]; then
    cp -a v6cord.png.c "$timestamp_ref"
fi

cmake -G Ninja \
    ${debug:+-DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-Og} \
    ${windows:+-DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH"} \
    ..

if cmp -s "$timestamp_ref" v6cord.png.c; then
    touch -r "$timestamp_ref" v6cord.png.c
fi

ninja

if [ ! -z "$windows" ]; then
    cp -uv ../../.github/libs/* .
fi
