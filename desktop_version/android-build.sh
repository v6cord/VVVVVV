#!/usr/bin/env bash
set -e
cd android-project
if [ -d ../../.github/resources/cmake ]; then
    echo "cmake.dir=$(realpath '../../.github/resources/cmake')" | tee -a local.properties
fi
if [ ! -z "$V6CORD_RELEASE" ]; then
    echo "$V6CORD_RELEASE" | base64 -d > app/v6cord-release.jks
fi
cat << EOF > keystore.properties
storePassword=$V6CORD_RELEASE_PASSWORD
keyPassword=$V6CORD_RELEASE_PASSWORD
keyAlias=v6cord
storeFile=v6cord-release.jks
EOF
sha256sum keystore.properties app/v6cord-release.jks || true
./gradlew assembleRelease
