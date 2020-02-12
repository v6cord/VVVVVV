#!/usr/bin/env bash
if [[ "$1" == "--prefix" ]]; then
    echo "/opt/devkitpro/portlibs/3ds"
elif [[ "$1" == "--cflags" ]]; then
    echo "-I/opt/devkitpro/portlibs/3ds/include/SDL2"
elif [[ "$1" == "--libs" ]]; then
    echo "-L/opt/devkitpro/portlibs/3ds/lib -lSDL2"
fi
