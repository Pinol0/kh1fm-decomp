#!/usr/bin/env python3
"""Export the map collision of a Kingdom Hearts area to Wavefront OBJ (see docs/area_formats.md).

Input: an area .bin (container; collision is entry 1, and entry 10 when present) or a raw
collision block. The game's y axis points down; the OBJ is written with y up.

Usage:
    python3 tools/collision2obj.py dh01.bin out.obj [--png preview.png]
    python3 tools/collision2obj.py collision.raw out.obj --raw
"""
import argparse
import struct
import zlib


def collision_blocks(data, raw):
    if raw:
        return [data]
    count = struct.unpack_from("<I", data, 0)[0]
    blocks = []
    for index in (1, 10):
        if index < count + 1:
            offset, size = struct.unpack_from("<2I", data, 8 + 8 * index)
            if size:
                blocks.append(data[offset:offset + size])
    return blocks


def parse(block):
    header = struct.unpack_from("<18I", block, 0)
    vert_off, vert_size = header[2], header[3]
    poly_off, poly_size = header[6], header[7]
    verts = [struct.unpack_from("<3f", block, vert_off + 12 * i) for i in range(vert_size // 12)]
    polys = []
    for i in range(poly_size // 20):
        v = struct.unpack_from("<4H", block, poly_off + 20 * i)
        flags = struct.unpack_from("<I", block, poly_off + 20 * i + 0x10)[0]
        polys.append(([x for x in v if x < 0xFFFE], flags))  # 0xFFFF and 0xFFFE: no 4th vertex
    return verts, polys


def write_obj(path, sets):
    with open(path, "w") as f:
        f.write("# Kingdom Hearts area collision, exported by tools/collision2obj.py\n")
        base = 0
        for n, (verts, polys) in enumerate(sets):
            f.write(f"o collision_{n}\n")
            for x, y, z in verts:
                f.write(f"v {x:.3f} {-y:.3f} {z:.3f}\n")
            for idx, flags in polys:
                if flags & 1:
                    continue  # ignored by the game
                f.write("f " + " ".join(str(base + i + 1) for i in reversed(idx)) + "\n")
            base += len(verts)


def write_png_top(path, sets, size=1024):
    """Top view (x, z) wireframe, brighter where higher."""
    pts = [v for verts, _ in sets for v in verts]
    xs, ys, zs = zip(*pts)
    x0, x1, z0, z1 = min(xs), max(xs), min(zs), max(zs)
    scale = (size - 20) / max(x1 - x0, z1 - z0)
    ymin, ymax = min(ys), max(ys)
    img = bytearray(size * size * 3)

    def plot(px, py, c):
        if 0 <= px < size and 0 <= py < size:
            o = (py * size + px) * 3
            img[o:o + 3] = bytes((c, c, min(255, c + 60)))

    def line(a, b, c):
        ax, ay = int((a[0] - x0) * scale) + 10, int((a[2] - z0) * scale) + 10
        bx, by = int((b[0] - x0) * scale) + 10, int((b[2] - z0) * scale) + 10
        n = max(abs(bx - ax), abs(by - ay), 1)
        for i in range(n + 1):
            plot(ax + (bx - ax) * i // n, ay + (by - ay) * i // n, c)

    for verts, polys in sets:
        for idx, flags in polys:
            if flags & 1:
                continue
            height = sum(-verts[i][1] for i in idx) / len(idx)
            c = int(70 + 185 * (height - (-ymax)) / max(ymax - ymin, 1))
            for i in range(len(idx)):
                line(verts[idx[i]], verts[idx[(i + 1) % len(idx)]], c)

    raw = b"".join(b"\0" + bytes(img[y * size * 3:(y + 1) * size * 3]) for y in range(size))

    def chunk(tag, body):
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--raw", action="store_true", help="input is a collision block, not an area .bin")
    ap.add_argument("--png", help="also write a top-view preview")
    args = ap.parse_args()

    data = open(args.input, "rb").read()
    sets = [parse(b) for b in collision_blocks(data, args.raw)]
    write_obj(args.output, sets)
    for n, (verts, polys) in enumerate(sets):
        print(f"collision {n}: {len(verts)} vertices, {len(polys)} polygons "
              f"({sum(1 for p in polys if len(p[0]) == 4)} quads, {sum(1 for p in polys if p[1] & 1)} ignored)")
    if args.png:
        write_png_top(args.png, sets)


if __name__ == "__main__":
    main()
