#!/usr/bin/env bash
cd "$(dirname "$0")"
source ./build.sh
if [[ ! -f data-zip/data.zip && ! -f data.zip ]]; then
    mkdir -p data-zip
    wget -O data-zip/data.zip https://thelettervsixtim.es/makeandplay/data.zip
fi
if [ ! -f data.zip ]; then
    ln -s data-zip/data.zip data.zip
fi
cpack
