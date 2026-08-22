RDR Bionic wrapper v21

Requirements on the Raspberry Pi:
  - AArch64 OS
  - /opt/rdr-pi-bionic/runtime/bin/linker64
  - Bionic runtime libraries in /opt/rdr-pi-bionic/runtime/lib64
  - cc, python3, patchelf, sha256sum

Install and build:
  cd wrapper/v21
  sudo sh install.sh
  sudo sh build.sh

Run:
  sh run.sh

Expected original hashes:
  librdr.so        0f288e2f9255d04277190bb5ffca742ec395d33a47fb305ddb22b9f0e904ef72
  libc++_shared.so f4e1e97c1943e60311e47e8b024d78f5b3b7229b3ccc65feb33af83d6025a670

V21 creates an isolated game namespace, then links only libc.so, libdl.so,
libm.so and libdl_android.so from the existing default namespace. This avoids
loading a second Bionic libc while keeping packaged libc++ private to RDR.

This is a diagnostic loader wrapper. It does not yet replace the Android
Java/activity environment required to enter the complete game loop.
