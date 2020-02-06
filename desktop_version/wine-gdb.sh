#!/bin/sh

# This is a small wrapper to append arguments to winedbg's gdb invocation.
# file VVVVVV-CE.exe loads symbols, tb SDL_main sets a one-time breakpoint at
# VVVVVV-CE's entrypoint, and c continues to there.
exec gdb "$@" -ex 'set confirm off' -ex 'file VVVVVV-CE.exe' -ex 'tb SDL_main' -ex 'c' -ex 'set confirm on'
