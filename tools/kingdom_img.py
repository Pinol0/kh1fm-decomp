#!/usr/bin/env python3
"""Read files from the KINGDOM.IDX / KINGDOM.IMG archive of a Kingdom Hearts (PS2) ISO.

Format as documented by OpenKH (https://github.com/OpenKH/OpenKh, Apache-2.0); this is an
independent implementation.

- KINGDOM.IDX: 0xE00 entries of {u32 hash, u32 compressed, u32 block, u32 length}.
- Blocks are 0x800 bytes, counted from the sector of SYSTEM.CNF.
- Names hash as h = (2 * h) ^ ((c << 16) % 69665) over their bytes.
- Compressed files are decoded from the end: last byte = escape key, the 3 bytes before it =
  decompressed size (big end first); key + distance + length copies length + 3 bytes,
  key + 0 is a literal key byte.

Usage:
    python3 tools/kingdom_img.py game.iso extract dh00_01.bin [out]
    python3 tools/kingdom_img.py game.iso list
"""
import struct
import sys

SECTOR = 0x800


def name_hash(name):
    h = 0
    for c in name.encode():
        h = ((2 * h) ^ ((c << 16) % 69665)) & 0xFFFFFFFF
    return h


def decompress(src):
    if len(src) < 4:
        return b""
    i = len(src) - 1
    key = src[i]
    size = src[i - 1] | (src[i - 2] << 8) | (src[i - 3] << 16)
    i -= 4
    out = bytearray(size)
    o = size - 1
    while o >= 0 and i >= 0:
        b = src[i]
        i -= 1
        if b == key and i >= 0:
            dist = src[i]
            i -= 1
            if dist > 0 and i >= 0:
                length = src[i] + 3
                i -= 1
                for _ in range(length):
                    if o < 0:
                        break
                    out[o] = out[o + dist] if o + dist < size else 0
                    o -= 1
                continue
        out[o] = b
        o -= 1
    return bytes(out)


class KingdomIso:
    def __init__(self, path):
        self.f = open(path, "rb")
        self.files = self._root_files()
        self.first_block = self.files["SYSTEM.CNF;1"][0]
        idx_block, idx_size = self.files["KINGDOM.IDX;1"]
        self.f.seek(idx_block * SECTOR)
        raw = self.f.read(idx_size)
        self.entries = {}
        for n in range(len(raw) // 16):
            h, comp, block, length = struct.unpack_from("<4I", raw, 16 * n)
            if h:
                self.entries[h] = (comp, block, length)

    def _root_files(self):
        self.f.seek(16 * SECTOR)
        pvd = self.f.read(SECTOR)
        root_lba, root_size = struct.unpack_from("<I", pvd, 156 + 2)[0], struct.unpack_from("<I", pvd, 156 + 10)[0]
        self.f.seek(root_lba * SECTOR)
        data = self.f.read(root_size)
        files, pos = {}, 0
        while pos < len(data):
            length = data[pos]
            if length == 0:
                pos = (pos // SECTOR + 1) * SECTOR
                continue
            lba, size = struct.unpack_from("<I", data, pos + 2)[0], struct.unpack_from("<I", data, pos + 10)[0]
            name = data[pos + 33:pos + 33 + data[pos + 32]].decode("ascii", "replace")
            files[name] = (lba, size)
            pos += length
        return files

    def read(self, name):
        entry = self.entries.get(name_hash(name))
        if entry is None:
            raise FileNotFoundError(name)
        comp, block, length = entry
        self.f.seek((self.first_block + block) * SECTOR)
        data = self.f.read(length)
        return decompress(data) if comp else data


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    iso = KingdomIso(sys.argv[1])
    if sys.argv[2] == "list":
        print(f"{len(iso.entries)} entries")
    elif sys.argv[2] == "extract":
        name = sys.argv[3]
        data = iso.read(name)
        out = sys.argv[4] if len(sys.argv) > 4 else name
        open(out, "wb").write(data)
        print(f"{name}: {len(data)} bytes -> {out}")


if __name__ == "__main__":
    main()
