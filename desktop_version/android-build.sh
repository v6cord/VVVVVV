#!/usr/bin/env bash
set -e
cd android-project
if [ -d ../../.github/resources/cmake ]; then
    echo "cmake.dir=$(realpath '../../.github/resources/cmake')" | tee -a local.properties
fi
./gradlew assembleDebug
