#!/usr/bin/env python3
"""Draft C for a function with m2c, using the project headers as context.
Usage: python3 tools/m2c.py <unit> <function>     e.g. tools/m2c.py game/area func_00112498"""
import subprocess
import sys
from pathlib import Path

unit, func = sys.argv[1], sys.argv[2]
src = Path("src") / f"{unit}.c"
ctx = subprocess.run(["cpp", "-P", "-Iinclude", "-Isrc", "-DM2CTX", "-D__attribute__(x)=", str(src)],
                     capture_output=True, text=True, check=True).stdout
Path("build/ctx.c").write_text(ctx)
asm = Path("asm/nonmatchings") / unit / f"{func}.s"
subprocess.run(["python3", "-m", "m2c.main", "-t", "mipsee-gcc-c", "--context", "build/ctx.c", str(asm)])
