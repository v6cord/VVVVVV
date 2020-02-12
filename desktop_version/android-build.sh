#!/usr/bin/env bash
set -e
cd android-project
if [ -d /usr/local/cmake ]; then
    echo 'cmake.dir=/usr/local/cmake' >> local.properties
fi
./gradlew assembleDebug
