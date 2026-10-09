#!/usr/bin/env python3
"""Run m2c on a function that uses jump tables: copies the function and its jump tables
(from the splat data asm) into one file, turning the table words into labels.
Usage: python3 tools/m2c_jtbl.py <func> [context.c]"""
import re
import subprocess
import sys
from pathlib import Path

func = sys.argv[1]
ctx = sys.argv[2] if len(sys.argv) > 2 else None
src = next(p for p in Path("asm").rglob("*.s") if f"glabel {func}\n" in p.read_text())
text = src.read_text()
body = re.search(rf"^nonmatching {func},.*?^endlabel {func}\n", text, re.M | re.S).group(0)
data = "".join(p.read_text() for p in Path("asm/data").glob("*.s"))
tables = ""
for jt in sorted(set(re.findall(r"jtbl_[0-9A-F]{8}", body))):
    m = re.search(rf"^(?:dlabel|jlabel|glabel) {jt}\n.*?(?=^(?:dlabel|glabel|jlabel) |\Z)", data, re.M | re.S)
    tables += m.group(0) + "\n"
targets = set(re.findall(r"\.word 0x00([0-9A-F]{6})", tables))
tables = re.sub(r"\.word 0x00([0-9A-F]{6})", r".word .L00\1", tables)
for t in targets:
    if f".L00{t}:" not in body:
        body = re.sub(rf"^(\s+/\* [0-9A-F]+ 00{t} )", rf"  .L00{t}:\n\1", body, flags=re.M)
out = Path("build") / f"{func}.jtbl.s"
out.write_text(".section .text\n" + body + "\n.section .rodata\n" + tables)
cmd = ["python3", "-m", "m2c.main", "-t", "mipsee-gcc-c"] + (["--context", ctx] if ctx else []) + [str(out)]
subprocess.run(cmd)
