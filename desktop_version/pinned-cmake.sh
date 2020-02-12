#!/usr/bin/env bash
wget https://github.com/Kitware/CMake/releases/download/v3.16.4/cmake-3.16.4-Linux-x86_64.sh \
    -q -O /tmp/cmake-install.sh

chmod u+x /tmp/cmake-install.sh
mkdir -p /usr/local/
/tmp/cmake-install.sh --skip-license --prefix=/usr/local/
rm /tmp/cmake-install.sh

