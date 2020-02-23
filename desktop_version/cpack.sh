#!/usr/bin/env bash
cd "$(dirname "$0")"
source ./build.sh
ln -s data-zip/data.zip data.zip || true
cpack
