#!/usr/bin/env bash
cd "$(dirname "$0")"
set -- -DDATA_ZIP_PATH="/usr/share/vvvvvv/data.zip"
source ./build.sh
ln -s data-zip/data.zip data.zip || true
cpack
mv -v VVVVVV-CE-*-Linux.deb VVVVVV-CE.deb
