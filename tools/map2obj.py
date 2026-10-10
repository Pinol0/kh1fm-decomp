#!/usr/bin/env python3
"""Export the visible map geometry of a Kingdom Hearts area (.bin entry 2) to OBJ.

Entry 2 holds a part count, a DMA table and one 0x80-byte bounding box per part. Each part is a
DMA "ref" to a VIF packet made of batches (see docs/area_formats.md):
    UNPACK V4-32 x1 @0     GIF tag (vertex count)
    STCYCL 3/1             vertices interleaved over 3 quadwords:
    UNPACK V4-32 @1        position (x, y, z, w); w != 0 closes a strip triangle
    UNPACK V4-8  @2        colour (RGBA, 0x80 = full)
    UNPACK V2-16 @3        texture coordinates (fixed point, 4096 = 1.0)
    DIRECT                 GS registers for the batch (texture)
    MSCNT                  run the VU1 program
The game's y axis points down; the OBJ is written with y up and vertex colours.

Usage: python3 tools/map2obj.py tw00_01.bin out.obj [--png preview.png]
"""
import argparse
import struct
import sys
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import vif  # noqa: E402


def batches(entry):
    count, table = struct.unpack_from("<2I", entry, 0)
    for part in range(count):
        tag, addr = struct.unpack_from("<2I", entry, table + 16 * (part + 1))
        qwc = tag & 0xFFFF
        pos = col = uv = None
        for _, cmd, num, imm, payload in vif.decode(entry, addr, addr + qwc * 16):
            if cmd < 0x60:
                if cmd in (0x14, 0x15, 0x17) and pos is not None:  # MSCAL / MSCALF / MSCNT
                    yield part, pos, col, uv
                    pos = col = uv = None
                continue
            n = num or 256
            slot = imm & 0x3FF
            fmt = vif.name(cmd)
            if fmt.startswith("UNPACK V4-32") and slot == 1:
                pos = [struct.unpack_from("<4f", payload, 16 * i) for i in range(n)]
            elif fmt.startswith("UNPACK V4-8") and slot == 2:
                col = [tuple(payload[4 * i:4 * i + 4]) for i in range(n)]
            elif fmt.startswith("UNPACK V2-16") and slot == 3:
                uv = [struct.unpack_from("<2h", payload, 4 * i) for i in range(n)]


def triangles(pos):
    for i in range(2, len(pos)):
        w = pos[i][3]
        if w == 0.0:
            continue
        yield (i - 2, i - 1, i) if w > 0 else (i - 1, i - 2, i)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--png", help="also write a top-view preview")
    args = ap.parse_args()

    data = open(args.input, "rb").read()
    off, size = struct.unpack_from("<2I", data, 0x18)
    entry = data[off:off + size]

    tris_out = []  # for the preview: (3 points, colour)
    nv = nt = nb = 0
    with open(args.output, "w") as f:
        f.write("# Kingdom Hearts area geometry, exported by tools/map2obj.py\n")
        last_part = -1
        for part, pos, col, uv in batches(entry):
            nb += 1
            if part != last_part:
                f.write(f"g part_{part}\n")
                last_part = part
            base = nv
            for i, (x, y, z, _) in enumerate(pos):
                r, g, b, _ = col[i] if col else (128, 128, 128, 128)
                f.write(f"v {x:.3f} {-y:.3f} {z:.3f} {min(r / 128, 1):.3f} {min(g / 128, 1):.3f} {min(b / 128, 1):.3f}\n")
            if uv:
                for s, t in uv:
                    f.write(f"vt {s / 4096:.5f} {1 - t / 4096:.5f}\n")
            for a, b2, c in triangles(pos):
                ia, ib, ic = base + a + 1, base + b2 + 1, base + c + 1
                if uv:
                    f.write(f"f {ia}/{ia} {ib}/{ib} {ic}/{ic}\n")
                else:
                    f.write(f"f {ia} {ib} {ic}\n")
                nt += 1
                if args.png:
                    tris_out.append(([pos[a], pos[b2], pos[c]], col[a] if col else (128, 128, 128, 128)))
            nv += len(pos)
    print(f"{nb} batches, {nv} vertices, {nt} triangles")
    if args.png:
        render_top(args.png, tris_out)


def render_top(path, tris, size=1024):
    """Top view (x, z), filled triangles with vertex colour, highest surface wins."""
    xs = [p[0] for t, _ in tris for p in t]
    zs = [p[2] for t, _ in tris for p in t]
    x0, z0 = min(xs), min(zs)
    scale = (size - 20) / max(max(xs) - x0, max(zs) - z0)
    img = bytearray(size * size * 3)
    depth = [1e30] * (size * size)
    for pts, (r, g, b, _) in tris:
        p = [((x - x0) * scale + 10, (z - z0) * scale + 10, y) for x, y, z, _ in pts]
        minx, maxx = max(int(min(q[0] for q in p)), 0), min(int(max(q[0] for q in p)) + 1, size - 1)
        miny, maxy = max(int(min(q[1] for q in p)), 0), min(int(max(q[1] for q in p)) + 1, size - 1)
        (ax, ay, az), (bx, by, bz), (cx, cy, cz) = p
        den = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
        if abs(den) < 1e-9:
            continue
        c = bytes((min(r * 2, 255), min(g * 2, 255), min(b * 2, 255)))
        for py in range(miny, maxy + 1):
            for px in range(minx, maxx + 1):
                l1 = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / den
                l2 = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / den
                l3 = 1 - l1 - l2
                if l1 < 0 or l2 < 0 or l3 < 0:
                    continue
                y = l1 * az + l2 * bz + l3 * cz  # game y points down: smaller is higher
                k = py * size + px
                if y < depth[k]:
                    depth[k] = y
                    img[3 * k:3 * k + 3] = c
    raw = b"".join(b"\0" + bytes(img[y * size * 3:(y + 1) * size * 3]) for y in range(size))

    def chunk(tag, body):
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


if __name__ == "__main__":
    main()
