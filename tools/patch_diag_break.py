#!/usr/bin/env python3
"""Patch a COPY of librdr.so so diagTerminate starts with AArch64 BRK.

The original game binary is never modified. The script finds the dynamic symbol
_Z13diagTerminatev, converts its virtual address to a file offset using PT_LOAD,
backs up the original 4-byte instruction, writes BRK #0 (0xd4200000), and emits
metadata for later inspection/restoration.
"""

from __future__ import annotations

import json
import struct
import sys
from pathlib import Path

PT_LOAD = 1
SHT_DYNSYM = 11
BRK = 0xD4200000
SYMBOL = b"_Z13diagTerminatev"


def u16(b: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<H", b, off)[0]


def u32(b: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<I", b, off)[0]


def u64(b: bytes | bytearray, off: int) -> int:
    return struct.unpack_from("<Q", b, off)[0]


def cstr(buf: bytes | bytearray, off: int) -> bytes:
    end = buf.find(b"\0", off)
    if end < 0:
        end = len(buf)
    return bytes(buf[off:end])


def main() -> int:
    if len(sys.argv) not in (2, 3):
        print(f"usage: {sys.argv[0]} LIBRDR_COPY [metadata.json]", file=sys.stderr)
        return 2

    path = Path(sys.argv[1])
    meta_path = Path(sys.argv[2]) if len(sys.argv) == 3 else path.with_suffix(path.suffix + ".diagbreak.json")
    data = bytearray(path.read_bytes())

    if data[:4] != b"\x7fELF" or data[4] != 2 or data[5] != 1:
        raise SystemExit("expected ELF64 little-endian input")

    e_phoff = u64(data, 32)
    e_shoff = u64(data, 40)
    e_phentsize = u16(data, 54)
    e_phnum = u16(data, 56)
    e_shentsize = u16(data, 58)
    e_shnum = u16(data, 60)

    loads = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        if u32(data, off) == PT_LOAD:
            loads.append({
                "offset": u64(data, off + 8),
                "vaddr": u64(data, off + 16),
                "filesz": u64(data, off + 32),
                "memsz": u64(data, off + 40),
            })

    found = None
    for i in range(e_shnum):
        sh = e_shoff + i * e_shentsize
        sh_type = u32(data, sh + 4)
        if sh_type != SHT_DYNSYM:
            continue

        sh_offset = u64(data, sh + 24)
        sh_size = u64(data, sh + 32)
        sh_link = u32(data, sh + 40)
        sh_entsize = u64(data, sh + 56) or 24

        str_sh = e_shoff + sh_link * e_shentsize
        str_off = u64(data, str_sh + 24)

        for sym_off in range(sh_offset, sh_offset + sh_size, sh_entsize):
            st_name = u32(data, sym_off)
            st_value = u64(data, sym_off + 8)
            st_size = u64(data, sym_off + 16)
            name = cstr(data, str_off + st_name)
            if name == SYMBOL:
                found = (st_value, st_size)
                break
        if found:
            break

    if not found:
        raise SystemExit(f"dynamic symbol {SYMBOL.decode()} not found")

    st_value, st_size = found
    file_off = None
    for seg in loads:
        if seg["vaddr"] <= st_value < seg["vaddr"] + seg["filesz"]:
            file_off = seg["offset"] + (st_value - seg["vaddr"])
            break

    if file_off is None:
        raise SystemExit(f"could not map symbol VA 0x{st_value:x} to file offset")
    if file_off + 4 > len(data):
        raise SystemExit("mapped file offset exceeds file size")

    original = bytes(data[file_off:file_off + 4])
    data[file_off:file_off + 4] = struct.pack("<I", BRK)
    path.write_bytes(data)

    meta = {
        "symbol": SYMBOL.decode(),
        "symbol_vaddr": f"0x{st_value:x}",
        "symbol_size": st_size,
        "file_offset": f"0x{file_off:x}",
        "original_bytes_le": original.hex(),
        "patched_bytes_le": struct.pack("<I", BRK).hex(),
        "instruction": "brk #0",
    }
    meta_path.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")

    print(f"patched {path}")
    print(f"symbol VA : 0x{st_value:x}")
    print(f"file off  : 0x{file_off:x}")
    print(f"original  : {original.hex()}")
    print(f"patched   : {struct.pack('<I', BRK).hex()}  (BRK #0)")
    print(f"metadata  : {meta_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
