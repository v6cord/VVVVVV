#!/usr/bin/env bash
cd "$(dirname "$0")"
unset DISPLAY
export XDG_DATA_HOME="$(pwd)/tests"
export USERPROFILE="$(pwd)/tests"
export SDL_AUDIODRIVER=dummy
export SDL_VIDEODRIVER=dummy
export XDG_RUNTIME_DIR="$(mktemp -d)"
function finish {
    rm -rf "$XDG_RUNTIME_DIR"
}
trap finish EXIT

if [ ! -f data.zip ]; then
    ln -s data-zip/data.zip data.zip || true
fi

source ./build.sh
set +e -o pipefail
failed=0
for testcase in "$XDG_DATA_HOME"/VVVVVV/levels/*.{vvvvvv,zip}; do
    testcase="$(basename "$testcase" .vvvvvv)"
    testcase="${testcase%.zip}"
    echo -n "$testcase -- "
    if output="$(timeout 5s ./VVVVVV-CE -p "$testcase" --headless --quiet 2>&1)"; then
        echo "PASS"
    else
        echo "$output"
        if command -v gdb >/dev/null 2>/dev/null; then
            echo -n "BACKTRACE: "
            timeout 5s gdb -batch -ex "run" -ex "bt" --args ./VVVVVV-CE -p "$testcase" --headless --quiet
        fi
        failed=1
    fi
done
exit "$failed"
