#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

source build.sh # source build.sh so we get its argument parsing for free!

export DISPLAY="${DISPLAY:-:0}" # if you're developing over ssh, set $DISPLAY

if [[ "$windows" == "1" ]]; then
    if command -v wine >&/dev/null; then
        exec wine ./VVVVVV-CE.exe "$@" # use wine if we have it
    else
        exec ./VVVVVV-CE.exe "$@" # assume we can execute PEs if we don't (either we're on Windows or the user has binfmt-misc set up)
    fi
else
    exec ./VVVVVV-CE "$@" # if we're not on windows, we can run it directly
fi
