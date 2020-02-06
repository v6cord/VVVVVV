#!/usr/bin/env bash
cd "$(dirname "$0")"
source ./build.sh
source ../download-data.sh
cpack
