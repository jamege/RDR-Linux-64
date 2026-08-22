import shutil
import struct
import sys

TARGETS = {
    "_Znwm", "_Znam", "_ZdlPv", "_ZdaPv",
    "_ZnwmSt11align_val_t", "_ZnamSt11align_val_t",
    "_ZdlPvSt11align_val_t", "_ZdaPvSt11align_val_t",
}

source, output = sys.argv[1:3]
shutil.copyfile(source, output)
data = bytearray(open(output, "rb").read())
if data[:6] != b"\x7fELF\x02\x01":
    raise SystemExit("expected ELF64 little-endian input")

e_shoff = struct.unpack_from("<Q", data, 0x28)[0]
e_shentsize, e_shnum, e_shstrndx = struct.unpack_from("<HHH", data, 0x3A)

def section(i):
    return struct.unpack_from("<IIQQQQIIQQ", data, e_shoff + i * e_shentsize)

def cstr(offset):
    end = data.index(0, offset)
    return data[offset:end].decode("utf-8", "replace")

sections = [section(i) for i in range(e_shnum)]
shstr = sections[e_shstrndx]
named = {cstr(shstr[4] + sh[0]): sh for sh in sections}
dynsym, dynstr = named[".dynsym"], named[".dynstr"]
found = set()

for off in range(dynsym[4], dynsym[4] + dynsym[5], dynsym[9]):
    st_name = struct.unpack_from("<I", data, off)[0]
    if not st_name:
        continue
    name = cstr(dynstr[4] + st_name)
    if name in TARGETS:
        data[off + 4] &= 0x0F  # STB_LOCAL; preserve symbol type.
        found.add(name)

missing = TARGETS - found
if missing:
    raise SystemExit("missing allocator symbols: " + ", ".join(sorted(missing)))

open(output, "wb").write(data)
print("localized:", ", ".join(sorted(found)))

