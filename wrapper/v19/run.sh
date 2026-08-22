#!/bin/sh
set -u
ROOT=/opt/rdr-pi-bionic
export LD_LIBRARY_PATH="$ROOT/runtime/lib64:$ROOT/game/lib"
"$ROOT/build/rdr_wrapper_v19"
rc=$?
echo "REAL EXIT CODE=$rc"
exit "$rc"

