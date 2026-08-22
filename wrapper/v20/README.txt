RDR Bionic wrapper v20

Requirements on the Raspberry Pi:
  - AArch64 OS
  - /opt/rdr-pi-bionic/runtime/bin/linker64
  - Bionic runtime libraries in /opt/rdr-pi-bionic/runtime/lib64
  - cc, patchelf, sha256sum

Install and build:
  cd wrapper/v20
  sudo sh install.sh
  sudo sh build.sh

Run:
  sh run.sh

Expected original hashes:
  librdr.so        0f288e2f9255d04277190bb5ffca742ec395d33a47fb305ddb22b9f0e904ef72
  libc++_shared.so f4e1e97c1943e60311e47e8b024d78f5b3b7229b3ccc65feb33af83d6025a670

V20 keeps RDR's original allocator symbols and loads librdr.so with its packaged
libc++_shared.so in an isolated Android linker namespace. This is intended to keep
the game and its C++ runtime in one allocator domain without exposing RDR's
operators to Android framework libraries.

This remains a diagnostic loader. Exit code 20 means the Android namespace APIs
were unavailable, 21 means namespace creation failed, and 30 means RDR failed to
load. It does not yet replace the Java/activity environment needed for the full
game loop.
