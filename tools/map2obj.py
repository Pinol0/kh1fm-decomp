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

Usage: python3 tools/map2obj.py tw00_01.bin out.obj [--img tw00_01.img] [--png preview.png]
With --img, every batch gets a material from its TEX0/CLAMP registers (see tools/areaimg.py).
"""
import argparse
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import areaimg  # noqa: E402
import vif  # noqa: E402


def batches(entry):
    count, table = struct.unpack_from("<2I", entry, 0)
    for part in range(count):
        tag, addr = struct.unpack_from("<2I", entry, table + 16 * (part + 1))
        qwc = tag & 0xFFFF
        pos = col = uv = gs = None
        for _, cmd, num, imm, payload in vif.decode(entry, addr, addr + qwc * 16):
            if cmd < 0x60:
                if cmd == 0x50:  # DIRECT: GIF A+D packet, TEX0_2 then CLAMP_2
                    gs = {struct.unpack_from("<Q", payload, 16 * i + 8)[0]: struct.unpack_from("<Q", payload, 16 * i)[0]
                          for i in range(1, len(payload) // 16)}
                elif cmd in (0x14, 0x15, 0x17) and pos is not None:  # MSCAL / MSCALF / MSCNT
                    yield part, pos, col, uv, gs
                    pos = col = uv = gs = None
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


TEX0_2, CLAMP_2 = 0x07, 0x09


class Materials:
    """One material per texture (TEX0) and repeat region (CLAMP); textures are written as PNG."""

    def __init__(self, img, outdir):
        self.img, self.outdir, self.known = img, outdir, {}

    def get(self, gs):
        tex0, clamp = gs[TEX0_2] & ((1 << 61) - 1), gs.get(CLAMP_2, 0)  # drop CLD (load control)
        key = (tex0, clamp)
        if key not in self.known:
            t, c = areaimg.tex0_fields(tex0), areaimg.clamp_fields(clamp)
            w, h, rgba = self.img.texture(tex0)
            name = f"t{t['tbp']:04x}_c{t['cbp']:04x}"
            su, sv = 1.0, 1.0  # REGION_REPEAT: the tile is cut out, uv scaled to repeat it
            if c["wms"] == 3 or c["wmt"] == 3:
                cw = c["minu"] + 1 if c["wms"] == 3 else w
                ch = c["minv"] + 1 if c["wmt"] == 3 else h
                x0 = c["maxu"] if c["wms"] == 3 else 0
                y0 = c["maxv"] if c["wmt"] == 3 else 0
                name += f"_r{x0}_{y0}_{cw}x{ch}"
                su, sv = w / cw, h / ch
                w, h, rgba = areaimg.crop(w, h, rgba, x0, y0, cw, ch)
            if self.outdir:
                areaimg.write_png(self.outdir / f"{name}.png", w, h, rgba)
            self.known[key] = (name, su, sv, (w, h, rgba))
        return self.known[key]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--img", help="the area's .img: write a .mtl and the textures next to the OBJ")
    ap.add_argument("--png", help="also write a top-view preview (textured with --img)")
    args = ap.parse_args()

    data = open(args.input, "rb").read()
    off, size = struct.unpack_from("<2I", data, 0x18)
    entry = data[off:off + size]
    out = Path(args.output)
    mats = None
    if args.img:
        texdir = out.parent / f"{out.stem}_tex"
        texdir.mkdir(parents=True, exist_ok=True)
        mats = Materials(areaimg.AreaImg(open(args.img, "rb").read()), texdir)

    tris_out = []  # for the preview: (3 points, 3 colours, 3 uvs, texture)
    nv = nt = nb = 0
    with open(out, "w") as f:
        f.write("# Kingdom Hearts area geometry, exported by tools/map2obj.py\n")
        if mats:
            f.write(f"mtllib {out.stem}.mtl\n")
        last_part, last_mat = -1, None
        for part, pos, col, uv, gs in batches(entry):
            nb += 1
            if part != last_part:
                f.write(f"g part_{part}\n")
                last_part = part
            su = sv = 1.0
            tex = None
            if mats and gs and TEX0_2 in gs:
                name, su, sv, tex = mats.get(gs)
                if name != last_mat:
                    f.write(f"usemtl {name}\n")
                    last_mat = name
            base = nv
            for i, (x, y, z, _) in enumerate(pos):
                r, g, b, _ = col[i] if col else (128, 128, 128, 128)
                f.write(f"v {x:.3f} {-y:.3f} {z:.3f} {min(r / 128, 1):.3f} {min(g / 128, 1):.3f} {min(b / 128, 1):.3f}\n")
            if uv:
                for s, t in uv:
                    f.write(f"vt {s / 4096 * su:.5f} {1 - t / 4096 * sv:.5f}\n")
            for a, b2, c in triangles(pos):
                ia, ib, ic = base + a + 1, base + b2 + 1, base + c + 1
                if uv:
                    f.write(f"f {ia}/{ia} {ib}/{ib} {ic}/{ic}\n")
                else:
                    f.write(f"f {ia} {ib} {ic}\n")
                nt += 1
                if args.png:
                    cs = [col[k] if col else (128, 128, 128, 128) for k in (a, b2, c)]
                    uvs = [(uv[k][0] / 4096 * su, uv[k][1] / 4096 * sv) for k in (a, b2, c)] if uv else None
                    tris_out.append(([pos[a], pos[b2], pos[c]], cs, uvs, tex))
            nv += len(pos)
    if mats:
        with open(out.with_suffix(".mtl"), "w") as f:
            for name, *_ in mats.known.values():
                f.write(f"newmtl {name}\nKd 1 1 1\nmap_Kd {out.stem}_tex/{name}.png\nmap_d {out.stem}_tex/{name}.png\n\n")
        print(f"{len(mats.known)} materials")
    print(f"{nb} batches, {nv} vertices, {nt} triangles")
    if args.png:
        render_top(args.png, tris_out)


def render_top(path, tris, size=1024):
    """Top view (x, z), filled triangles (textured when known), highest surface wins."""
    xs = [p[0] for t, *_ in tris for p in t]
    zs = [p[2] for t, *_ in tris for p in t]
    x0, z0 = min(xs), min(zs)
    scale = (size - 20) / max(max(xs) - x0, max(zs) - z0)
    img = bytearray(size * size * 3)
    depth = [1e30] * (size * size)
    for pts, cols, uvs, tex in tris:
        p = [((x - x0) * scale + 10, (z - z0) * scale + 10, y) for x, y, z, _ in pts]
        minx, maxx = max(int(min(q[0] for q in p)), 0), min(int(max(q[0] for q in p)) + 1, size - 1)
        miny, maxy = max(int(min(q[1] for q in p)), 0), min(int(max(q[1] for q in p)) + 1, size - 1)
        (ax, ay, az), (bx, by, bz), (cx, cy, cz) = p
        den = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
        if abs(den) < 1e-9:
            continue
        for py in range(miny, maxy + 1):
            for px in range(minx, maxx + 1):
                l1 = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / den
                l2 = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / den
                l3 = 1 - l1 - l2
                if l1 < 0 or l2 < 0 or l3 < 0:
                    continue
                y = l1 * az + l2 * bz + l3 * cz  # game y points down: smaller is higher
                k = py * size + px
                if y >= depth[k]:
                    continue
                cr, cg, cb = (l1 * cols[0][i] + l2 * cols[1][i] + l3 * cols[2][i] for i in range(3))
                if tex and uvs:
                    w, h, rgba = tex
                    u = l1 * uvs[0][0] + l2 * uvs[1][0] + l3 * uvs[2][0]
                    v = l1 * uvs[0][1] + l2 * uvs[1][1] + l3 * uvs[2][1]
                    o = 4 * (int(v * h) % h * w + int(u * w) % w)
                    if rgba[o + 3] < 0x40:
                        continue  # alpha-tested texel
                    t = rgba[o:o + 3]
                    c = bytes((min(int(t[0] * cr / 128), 255), min(int(t[1] * cg / 128), 255), min(int(t[2] * cb / 128), 255)))
                else:
                    c = bytes((min(int(cr * 2), 255), min(int(cg * 2), 255), min(int(cb * 2), 255)))
                depth[k] = y
                img[3 * k:3 * k + 3] = c
    areaimg.write_png(path, size, size, bytes(img), channels=3)


if __name__ == "__main__":
    main()
