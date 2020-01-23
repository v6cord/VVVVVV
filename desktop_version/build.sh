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

cmake -G Ninja \
    ${debug:+-DCMAKE_BUILD_TYPE=Debug} \
    ${windows:+-DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH"} \
    ..

ninja

if [ ! -z "$windows" ]; then
    cp -uv ../../.github/libs/* .
fi
