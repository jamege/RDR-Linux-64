#!/bin/sh
set -eu

ROOT=/opt/rdr-pi-bionic
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD="$ROOT/build"
STUB="$BUILD/linkstub-v22"

mkdir -p "$BUILD" "$STUB"

cc -nostdlib -fPIC -shared -Wl,-soname,libdl.so \
  "$HERE/src/linkstub_libdl.c" -o "$STUB/libdl.so"
cc -nostdlib -fPIC -shared -Wl,-soname,libc.so \
  "$HERE/src/linkstub_libc.c" -o "$STUB/libc.so"

cc -nostdlib -ffreestanding -fno-builtin -fno-stack-protector -fPIE -c \
  "$HERE/src/rdr_wrapper_v22.c" -o "$BUILD/rdr_wrapper_v22.o"

cc -nostdlib -pie -Wl,-e,_start \
  -Wl,--dynamic-linker="$ROOT/runtime/bin/linker64" \
  -Wl,-rpath,"$ROOT/runtime/lib64:$ROOT/game/lib" \
  -Wl,--no-as-needed "$BUILD/rdr_wrapper_v22.o" \
  -L"$STUB" -ldl -lc -o "$BUILD/rdr_wrapper_v22"

patchelf --add-needed libm.so "$BUILD/rdr_wrapper_v22"
echo "built $BUILD/rdr_wrapper_v22"
