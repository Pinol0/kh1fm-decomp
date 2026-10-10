#!/usr/bin/env python3
"""Decode the textures of a Kingdom Hearts area (<area>.img), as the game uploads them to GS memory.

Header (u32 offset/size pairs, offsets from the start of the file), read by func_00101048:
    0x00 / 0x04   records of 0xA0 bytes (copied to D_0029B420)
    0x08          info: 16 page formats (u8, 0x13 = PSMT8, 0x14 = PSMT4), then at +0x14 / +0x16
                  the number of 16-colour CLUTs and the end of the 256-colour ones (u16 each)
    0x10 / 0x14   CLUTs, one GS block (0x100 bytes) each, uploaded at CBP 0x3E00 + index:
                  16 colours as 8x2 PSMCT32, then 256 colours as 16x16 PSMCT32 every 4 blocks
    0x18 / 0x1C   texture pages, 0x10000 bytes each, uploaded at TBP 0x2600 + 0x100 * page
                  (PSMT4 512x256 or PSMT8 256x256); after the 16 pages, 0x40000 bytes of
                  PSMT8H 256x1024 uploaded at TBP 0 (four 256x256 textures, TBP 0/0x400/0x800/0xC00)
    0x20 / 0x24   records of 2064 bytes (func_00100F70)

Every texture is read back with the PSM and buffer width it was uploaded with, so the data is
linear in the file: no GS swizzling is needed.

Usage: python3 tools/areaimg.py tw00_01.img outdir      (writes every page with its first CLUT)
"""
import struct
import sys
import zlib
from pathlib import Path

PSMT8, PSMT4, PSMT8H = 0x13, 0x14, 0x1B


def tex0_fields(tex0):
    return {
        "tbp": tex0 & 0x3FFF, "tbw": (tex0 >> 14) & 0x3F, "psm": (tex0 >> 20) & 0x3F,
        "tw": 1 << ((tex0 >> 26) & 0xF), "th": 1 << ((tex0 >> 30) & 0xF),
        "tcc": (tex0 >> 34) & 1, "tfx": (tex0 >> 35) & 3, "cbp": (tex0 >> 37) & 0x3FFF,
        "cpsm": (tex0 >> 51) & 0xF, "csm": (tex0 >> 55) & 1, "csa": (tex0 >> 56) & 0x1F,
    }


def clamp_fields(clamp):
    return {
        "wms": clamp & 3, "wmt": (clamp >> 2) & 3, "minu": (clamp >> 4) & 0x3FF,
        "maxu": (clamp >> 14) & 0x3FF, "minv": (clamp >> 24) & 0x3FF, "maxv": (clamp >> 34) & 0x3FF,
    }


class AreaImg:
    def __init__(self, data):
        self.data = data
        h = struct.unpack_from("<10I", data, 0)
        self.info, self.clut_off, self.tex_off = h[2], h[4], h[6]
        self.formats = data[self.info:self.info + 16]

    def clut(self, cbp, colours):
        base = self.clut_off + 0x100 * (cbp - 0x3E00)
        raw = [self.data[base + 4 * i:base + 4 * i + 4] for i in range(colours)]
        if colours == 256:  # CSM1: entries 8-15 and 16-23 of every 32 are swapped
            raw = [raw[(i & ~0x18) | ((i & 8) << 1) | ((i & 0x10) >> 1)] for i in range(256)]
        return [bytes((min(r, 255), min(g, 255), min(b, 255), min(a * 2, 255))) for r, g, b, a in raw]

    def texture(self, tex0):
        """RGBA bytes (tw * th * 4) of the texture a TEX0 register value points at."""
        t = tex0_fields(tex0)
        w, h, psm = t["tw"], t["th"], t["psm"]
        if psm == PSMT8H:
            base, stride = self.tex_off + 0x100000 + 0x40 * t["tbp"], 256
        else:
            page = (t["tbp"] - 0x2600) // 0x100
            base = self.tex_off + 0x10000 * page
            stride = 512 if psm == PSMT4 else 256
        pal = self.clut(t["cbp"], 16 if psm == PSMT4 else 256)
        out = bytearray()
        for y in range(h):
            if psm == PSMT4:
                row = self.data[base + y * stride // 2:base + (y * stride + w) // 2]
                for b in row:
                    out += pal[b & 0xF] + pal[b >> 4]
            else:
                for b in self.data[base + y * stride:base + y * stride + w]:
                    out += pal[b]
        return w, h, bytes(out)


def crop(w, h, rgba, x0, y0, cw, ch):
    rows = [rgba[4 * ((y0 + y) * w + x0):4 * ((y0 + y) * w + x0 + cw)] for y in range(ch)]
    return cw, ch, b"".join(rows)


def write_png(path, w, h, rgba, channels=4):
    raw = b"".join(b"\0" + rgba[y * w * channels:(y + 1) * w * channels] for y in range(h))

    def chunk(tag, body):
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body))

    colour_type = 6 if channels == 4 else 2
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, colour_type, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    img = AreaImg(open(sys.argv[1], "rb").read())
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    for page, fmt in enumerate(img.formats):
        tbw, tw = (8, 9) if fmt == PSMT4 else (4, 8)
        tex0 = (0x2600 + 0x100 * page) | tbw << 14 | fmt << 20 | tw << 26 | 8 << 30 | 0x3E00 << 37
        write_png(out / f"page{page:02}.png", *img.texture(tex0))
    print(f"{len(img.formats)} pages -> {out}")


if __name__ == "__main__":
    main()
