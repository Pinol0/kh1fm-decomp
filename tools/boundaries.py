#!/usr/bin/env python3
"""Guess object-file boundaries in .text from link padding.

The linker aligns every object's .text to 8 bytes, so a file whose code ends on a
4-byte boundary is followed by one padding nop. A function ending in `jr`/`j` + delay
slot, followed by one or more zero words before an 8-aligned function start, is a
strong file boundary candidate (boundaries that need no padding stay invisible). Usage: python3 tools/boundaries.py [start] [end]  (vram, hex)
"""
import re
import struct
import sys
from pathlib import Path

VRAM = 0x100000
rom = Path("SLPS_251.98.rom").read_bytes()
lo = int(sys.argv[1], 16) if len(sys.argv) > 1 else VRAM
hi = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x272C70
funcs = sorted(int(m.group(1), 16) for p in Path("asm").rglob("*.s")
               for m in re.finditer(r"^glabel func_([0-9A-F]{8})$", p.read_text(), re.M))


def word(addr):
    return struct.unpack_from("<I", rom, addr - VRAM)[0]


def is_jump(w):
    return w == 0x03E00008 or (w >> 26) == 0b000010  # jr $ra / j


for a in funcs:
    if not (lo < a <= hi) or a % 8:
        continue
    k = 0
    while word(a - 4 * (k + 1)) == 0 and k < 64:
        k += 1
    # nop delay slot + padding, or a non-nop delay slot + padding
    if (k >= 2 and is_jump(word(a - 4 * (k + 1)))) or (k >= 1 and is_jump(word(a - 4 * (k + 2)))):
        print(f"{a:08X}")
