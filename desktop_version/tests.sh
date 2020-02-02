#!/usr/bin/env bash
cd "$(dirname "$0")"
unset DISPLAY
export XDG_DATA_HOME="$(pwd)/tests"
export USERPROFILE="$(pwd)/tests"
export SDL_AUDIODRIVER=dummy
export XDG_RUNTIME_DIR="$(mktemp -d)"
function finish {
    rm -f "$XDG_RUNTIME_DIR"
}
trap finish EXIT

source ./build.sh
if [ ! -f data.zip ]; then
    wget https://thelettervsixtim.es/makeandplay/data.zip
fi
set +e -o pipefail
failed=0
for testcase in "$XDG_DATA_HOME"/VVVVVV/levels/*.vvvvvv; do
    testcase="$(basename "$testcase" .vvvvvv)"
    echo -n "$testcase -- "
    if output="$(./VVVVVV-CE -p "$testcase" --headless --quiet 2>&1)"; then
        echo "PASS"
    else
        echo "$output"
        echo -n "BACKTRACE: "
        gdb -batch -ex "run" -ex "bt" --args ./VVVVVV-CE -p "$testcase" --headless --quiet
        failed=1
    fi
done
exit "$failed"
