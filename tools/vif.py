#!/usr/bin/env python3
"""Minimal PS2 VIF command stream decoder (enough for the map geometry packets)."""
import struct

UNPACK_FMT = {  # vn, vl -> (components, bits)
    (0, 0): (1, 32), (0, 1): (1, 16), (0, 2): (1, 8),
    (1, 0): (2, 32), (1, 1): (2, 16), (1, 2): (2, 8),
    (2, 0): (3, 32), (2, 1): (3, 16), (2, 2): (3, 8),
    (3, 0): (4, 32), (3, 1): (4, 16), (3, 2): (4, 8), (3, 3): (4, 5),
}
NAMES = {0x00: "NOP", 0x01: "STCYCL", 0x02: "OFFSET", 0x03: "BASE", 0x04: "ITOP", 0x05: "STMOD",
         0x06: "MSKPATH3", 0x07: "MARK", 0x10: "FLUSHE", 0x11: "FLUSH", 0x13: "FLUSHA", 0x14: "MSCAL",
         0x15: "MSCALF", 0x17: "MSCNT", 0x20: "STMASK", 0x30: "STROW", 0x31: "STCOL", 0x4A: "MPG",
         0x50: "DIRECT", 0x51: "DIRECTHL"}


def decode(data, start=0, end=None):
    """Yield (offset, cmd, num, imm, payload) for each VIF code."""
    end = len(data) if end is None else end
    pos = start
    cl, wl = 1, 1
    while pos + 4 <= end:
        code = struct.unpack_from("<I", data, pos)[0]
        imm, num, cmd = code & 0xFFFF, (code >> 16) & 0xFF, (code >> 24) & 0x7F
        pos += 4
        payload = b""
        if cmd >= 0x60:  # UNPACK
            vn, vl = (cmd >> 2) & 3, cmd & 3
            comps, bits = UNPACK_FMT[(vn, vl)]
            n = num or 256
            if wl <= cl:
                count = n
            else:
                count = n  # filling write not used here
            size = (comps * bits * count + 31) // 32 * 4 if bits != 5 else 2 * count
            size = (size + 3) & ~3
            payload = data[pos:pos + size]
            pos += size
        elif cmd == 0x01:
            cl, wl = imm & 0xFF, imm >> 8
        elif cmd in (0x20,):
            payload = data[pos:pos + 4]; pos += 4
        elif cmd in (0x30, 0x31):
            payload = data[pos:pos + 16]; pos += 16
        elif cmd == 0x4A:
            n = num or 256
            payload = data[pos:pos + 8 * n]; pos += 8 * n
        elif cmd in (0x50, 0x51):
            n = imm or 65536
            payload = data[pos:pos + 16 * n]; pos += 16 * n
        yield pos - 4 - len(payload), cmd, num, imm, payload


def name(cmd):
    if cmd >= 0x60:
        vn, vl = (cmd >> 2) & 3, cmd & 3
        comps, bits = UNPACK_FMT[(vn, vl)]
        return f"UNPACK V{comps}-{bits}" + (" (masked)" if cmd & 0x10 else "")
    return NAMES.get(cmd, f"cmd{cmd:02X}")
