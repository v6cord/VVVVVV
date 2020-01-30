#!/usr/bin/env bash
cd "$(dirname "$0")"
unset DISPLAY
export XDG_DATA_HOME="$(pwd)/tests"
export USERPROFILE="$(pwd)/tests"
export SDL_AUDIODRIVER=dummy
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
        failed=1
    fi
done
exit "$failed"
