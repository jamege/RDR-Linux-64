#!/bin/bash
set -euo pipefail

ROOT=${RDR_ROOT:-/opt/rdr-pi-bionic}
LOG="$ROOT/rdr-bionic-v15-gdb.log"
PROBE="$ROOT/build/rdr_probe_v15"

if [ ! -x "$PROBE" ]; then
    echo "ERROR: $PROBE missing. Run scripts/build-v15.sh first."
    exit 2
fi

export LD_LIBRARY_PATH="$ROOT/runtime/lib64:$ROOT/game/lib"
export ANDROID_DATA="$ROOT/android-data"
export ANDROID_ROOT="$ROOT/android-root"
mkdir -p "$ANDROID_DATA" "$ANDROID_ROOT"

GDBCMD=$(mktemp)
trap 'rm -f "$GDBCMD"' EXIT

cat >"$GDBCMD" <<'EOF'
set pagination off
set confirm off
set breakpoint pending on
handle SIGTRAP stop print nopass
run
printf "\n=== V15 STOP ===\n"
bt
printf "\n=== REGISTERS ===\n"
info registers pc lr x0 x1 x2 x3 x4 x5 x6 x7
printf "\n=== CURRENT INSTRUCTIONS ===\n"
x/12i $pc-16
printf "\n=== CALLER INSTRUCTIONS ===\n"
frame 1
x/16i $pc-32
printf "\n=== MAPPINGS ===\n"
info proc mappings
quit
EOF

set +e
gdb -q -batch -x "$GDBCMD" --args "$PROBE" 2>&1 | tee "$LOG"
RC=${PIPESTATUS[0]}
set -e

echo
echo "gdb rc=$RC"
echo "log: $LOG"
echo
echo "Expected useful result: PC should stop at BRK #0 inside copied librdr.so,"
echo "with frame #1 showing the exact function that called diagTerminate()."
exit "$RC"
