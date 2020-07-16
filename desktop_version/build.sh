#!/usr/bin/env bash
set -e

if [ -z "$VVVVVV_CE_OUT_OF_TREE_BUILD" ]; then
    cd "$(dirname "$0")"
fi

if [ ! -z "$VVVVVV_CE_ANDROID_BUILD" ]; then
    exec ./android-build.sh
fi

CC="${CC:-cc}"
if [ ! -z "$VVVVVV_CE_SWITCH_BUILD" ]; then
    CC="aarch64-none-elf-gcc"
fi
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

if [ -e vce.zip.c ]; then
    cp -a vce.zip.c "$timestamp_ref"
fi

cmake -G Ninja \
    ${debug:+-DCMAKE_BUILD_TYPE=Debug} \
    ${debug:--DCMAKE_BUILD_TYPE=RelWithDebInfo} \
    ${windows:+-DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain.cmake -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH"} \
    ${VVVVVV_CE_SWITCH_BUILD:+-DCMAKE_TOOLCHAIN_FILE=/usr/local/share/switch-cmake/DevkitA64Libnx.cmake} \
    ${VVVVVV_CE_SWITCH_BUILD:+-DCMAKE_MODULE_PATH=/usr/local/share/switch-cmake/cmake} \
    "$@" \
    ..

if cmp -s "$timestamp_ref" vce.zip.c; then
    touch -r "$timestamp_ref" vce.zip.c
fi

ninja ${verbose:+-v}
