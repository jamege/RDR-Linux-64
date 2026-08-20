#!/bin/bash
set -euo pipefail

ROOT=${RDR_ROOT:-/opt/rdr-pi-bionic}
REPO=${RDR_REPO:-$PWD}
BUILD="$ROOT/build"
WORK="$ROOT/work/v15"
STUB="$BUILD/linkstub"

mkdir -p "$BUILD" "$WORK" "$STUB"

if [ ! -f "$ROOT/game/lib/librdr.so" ]; then
    echo "ERROR: $ROOT/game/lib/librdr.so not found"
    exit 2
fi

if [ ! -x "$ROOT/runtime/bin/linker64" ]; then
    echo "ERROR: Bionic linker64 missing at $ROOT/runtime/bin/linker64"
    exit 3
fi

cp -f "$ROOT/game/lib/librdr.so" "$WORK/librdr-diagbreak.so"
python3 "$REPO/tools/patch_diag_break.py" \
  "$WORK/librdr-diagbreak.so" \
  "$WORK/librdr-diagbreak.json"

cc -nostdlib -fPIC -shared \
  -Wl,-soname,libdl.so \
  "$REPO/src/linkstub_libdl.c" \
  -o "$STUB/libdl.so"

cc -nostdlib -ffreestanding -fno-builtin -fno-stack-protector -fPIE -c \
  "$REPO/src/bionic_probe_v15.c" \
  -o "$BUILD/rdr_probe_v15.o"

cc -nostdlib -pie \
  -Wl,--dynamic-linker="$ROOT/runtime/bin/linker64" \
  -Wl,-rpath,"$ROOT/runtime/lib64:$ROOT/game/lib" \
  -Wl,--no-as-needed \
  "$BUILD/rdr_probe_v15.o" \
  -L"$STUB" -ldl \
  -o "$BUILD/rdr_probe_v15"

# v9 proved Bionic libc/libm must be loaded at process startup for TLS.
patchelf --add-needed libc.so "$BUILD/rdr_probe_v15"
patchelf --add-needed libm.so "$BUILD/rdr_probe_v15"

echo
echo "=== v15 build complete ==="
file "$BUILD/rdr_probe_v15"
echo
echo "Patch metadata:"
cat "$WORK/librdr-diagbreak.json"
