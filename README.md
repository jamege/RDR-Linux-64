# RDR Linux 64

Experimental ARM64 Linux wrapper/loader work for the Android ARM64 build of Red Dead Redemption.

## Current status

The Bionic runtime route is proven on Raspberry Pi OS ARM64:

- Android `linker64` runs on the Pi.
- Original `libc++_shared.so` loads under Bionic.
- `libplaycore.so`, `libac0185.so`, `libandroidx.graphics.path.so`, `libsqlcipher.so`, and `libbacktrace-native.so` all load.
- `librdr.so` reaches load-time code but currently trips `SIGTRAP` through `diagTerminate()` while Android Codec2 constructor code is executing.

The next diagnostic is v15: patch only a copied `librdr.so` so the first instruction of `_Z13diagTerminatev` becomes an AArch64 `BRK`. Running that copy under GDB should stop exactly at entry to `diagTerminate()` and reveal the real caller before `tgkill(SIGTRAP)` happens.

No game entrypoint is called yet.

## Repository policy

Do not commit proprietary game assets or binaries. Keep game files locally under `game/` (ignored by Git) and point the scripts at them.
