#!/usr/bin/env python3
"""Side-by-side diff of one function: expected (target) vs built (base) object.
Usage: python3 tools/diff.py <unit> <function> [-a]   (-a: show all lines, not only differences)"""
import re
import subprocess
import sys

unit, func = sys.argv[1], sys.argv[2]
show_all = "-a" in sys.argv


def disasm(obj):
    out = subprocess.run(["mips-linux-gnu-objdump", "-dr", "--no-show-raw-insn", "-Mreg-names=numeric",
                          f"--disassemble={func}", obj], capture_output=True, text=True).stdout
    lines = []
    for line in out.splitlines():
        m = re.match(r"\s*[0-9a-f]+:\s+(.*)", line)
        if m:
            text = re.sub(r"\s+", " ", m.group(1)).strip()
            if re.match(r"R_MIPS_\w+\s", text):  # relocation line: attach to previous instruction
                lines[-1] += "  <" + text.split()[-1] + ">"
            else:
                lines.append(re.sub(r"[0-9a-f]+ <[^>]+>", "<branch>", text))
    return lines


a = disasm(f"expected/{unit}.o")
b = disasm(f"build/src/{unit}.o")
n = max(len(a), len(b))
diffs = 0
def same(x, y):
    # A call or address of a function in the same file is relocated against the section
    # (<.text>, with the offset in the instruction) in the built object: same final bytes.
    if x == y:
        return True
    if "<.text>" in y and "<" in x:
        return re.sub(r"\s*<[^>]*>$", "", x).split(",")[0] == re.sub(r"\s*<[^>]*>$", "", y).split(",")[0]
    return False


for i in range(n):
    x = a[i] if i < len(a) else ""
    y = b[i] if i < len(b) else ""
    mark = " " if same(x, y) else "|"
    diffs += mark == "|"
    if show_all or mark == "|":
        print(f"{i*4:4x} {x:<48} {mark} {y}")
print(f"{diffs} differing lines (expected {len(a)}, built {len(b)} instructions)")
