#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

export WINE_GDB="$(realpath wine-gdb.sh)" # we have to do this before build.sh or we'll be in the wrong directory
export debug=1

source build.sh # source build.sh so we get its argument parsing for free!

export DISPLAY="${DISPLAY:-:0}" # if you're developing over ssh, set $DISPLAY

if [[ "$windows" == "1" ]] && command -v wine >&/dev/null ; then # if we're on wine
    exec winedbg --gdb "VVVVVV-CE.exe" "$@" # use winedbg to automatically setup a gdb server. this doesn't work great, but it's better than nothing
else
    exec gdb ./VVVVVV-CE "$@" # if we're not on wine, we can run gdb directly
fi
