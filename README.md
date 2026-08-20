# RDR Linux 64

Experimental ARM64 Linux wrapper/loader work for the Android ARM64 build of Red Dead Redemption.

## Current status

The Bionic runtime route is proven on Raspberry Pi OS ARM64:

- Android `linker64` runs on the Pi.
- Original `libc++_shared.so` loads under Bionic.
- `libplaycore.so`, `libac0185.so`, `libandroidx.graphics.path.so`, `libsqlcipher.so`, and `libbacktrace-native.so` all load.
- `librdr.so` reaches load-time code but currently trips `SIGTRAP` through `diagTerminate()` while Android Codec2 constructor code is executing.

The earlier constructor-disabled test still trapped, so v15 does not guess at constructor removal or symbol interposition. Instead it patches only a copied `librdr.so`: the first instruction of `_Z13diagTerminatev` is replaced with AArch64 `BRK #0`. Under GDB that should stop exactly at entry to `diagTerminate()` and show the real caller before `tgkill(SIGTRAP)` happens.

No game entrypoint is called yet.

## v15 files

- `tools/patch_diag_break.py` — finds `_Z13diagTerminatev` in the ELF and patches a copy with `BRK #0`.
- `src/bionic_probe_v15.c` — tiny freestanding Bionic `dlopen()` probe.
- `src/linkstub_libdl.c` — link-time stub so the probe can be built without glibc.
- `scripts/build-v15.sh` — creates the patched copy and builds the Bionic probe.
- `scripts/run-v15-gdb.sh` — runs the probe under GDB and dumps backtrace, registers, instructions and mappings.

## Run v15 on the Pi

Clone and switch to the diagnostic branch:

```bash
git clone https://github.com/jamege/RDR-Linux-64.git
cd RDR-Linux-64
git checkout wrapper-v15
chmod +x scripts/*.sh tools/*.py
```

The existing working runtime is expected at `/opt/rdr-pi-bionic`, including:

```text
/opt/rdr-pi-bionic/runtime/bin/linker64
/opt/rdr-pi-bionic/runtime/lib64/
/opt/rdr-pi-bionic/game/lib/librdr.so
```

Build and run:

```bash
RDR_REPO="$PWD" scripts/build-v15.sh
scripts/run-v15-gdb.sh
```

The GDB log is written to:

```text
/opt/rdr-pi-bionic/rdr-bionic-v15-gdb.log
```

The useful result is a stop at the injected `BRK #0` inside the copied library with frame `#1` revealing the exact caller of `diagTerminate()`.

## Repository policy

Do not commit proprietary game assets or binaries. Keep game files locally under `game/` (ignored by Git) and point the scripts at them.
