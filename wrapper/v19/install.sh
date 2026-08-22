#!/bin/sh
set -eu

ROOT=/opt/rdr-pi-bionic
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
GAME="$ROOT/game/lib"

mkdir -p "$GAME"
cp "$HERE/payload/librdr.so" "$GAME/librdr.so"
cp "$HERE/payload/libc++_shared.so" "$GAME/libc++_shared.so"
python3 "$HERE/tools/localize_allocators.py" \
  "$GAME/librdr.so" "$GAME/librdr-local.so"

echo "installed game libraries in $GAME"
sha256sum "$GAME/librdr.so" "$GAME/libc++_shared.so" "$GAME/librdr-local.so"

