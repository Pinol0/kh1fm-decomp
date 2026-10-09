#!/usr/bin/env python3
"""List which functions reference which strings, by tracking lui/addiu pairs.

Used to locate library code (SDK error messages name their function) and file
boundaries. Function starts come from the splat asm (glabel lines).
Usage: python3 tools/strrefs.py [regex]
"""
import re
import struct
import sys
from bisect import bisect_right
from pathlib import Path

import rabbitizer

VRAM = 0x100000
TEXT_END = 0x172C70
rom = Path("SLPS_251.98.rom").read_bytes()

funcs = sorted(int(m.group(1), 16) for p in Path("asm").rglob("*.s")
               for m in re.finditer(r"^glabel func_([0-9A-F]{8})$", p.read_text(), re.M))


def c_string(addr):
    off = addr - VRAM
    if not (TEXT_END <= off < len(rom)):
        return None
    end = rom.find(b"\0", off, off + 256)
    if end <= off:
        return None
    s = rom[off:end]
    if all(32 <= b < 127 or b in (9, 10) for b in s) and len(s) >= 4:
        return s.decode()
    return None


def refs():
    hi = {}
    for off in range(0, TEXT_END, 4):
        w = struct.unpack_from("<I", rom, off)[0]
        ins = rabbitizer.Instruction(w, vram=VRAM + off, category=rabbitizer.InstrCategory.R5900)
        name = ins.getOpcodeName()
        if name == "lui":
            hi[ins.rt] = ins.getProcessedImmediate() << 16
        elif name in ("addiu", "ori") and ins.rs in hi:
            imm = ins.getProcessedImmediate()
            yield VRAM + off, (hi[ins.rs] + imm) & 0xFFFFFFFF
        if ins.isJump() and not name.startswith("jal"):
            hi.clear()


pattern = re.compile(sys.argv[1] if len(sys.argv) > 1 else ".")
for pc, addr in refs():
    s = c_string(addr)
    if s and pattern.search(s):
        f = funcs[bisect_right(funcs, pc) - 1] if funcs and pc >= funcs[0] else None
        print(f"func_{f:08X}  {s!r}")
