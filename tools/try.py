#!/usr/bin/env python3
"""Try alternative bodies for one function and count differing instructions for each.
Usage: python3 tools/try.py <unit> <function> <variants file>
The variants file holds complete function definitions separated by lines containing only ----.
The source file is restored afterwards."""
import re
import subprocess
import sys
from pathlib import Path

unit, func, variants = sys.argv[1:4]
src = Path("src") / f"{unit}.c"
orig = src.read_text()
m = re.search(rf"^[^\n;]*\b{func}\(", orig, re.M)
start = m.start()
depth, i = 0, orig.index("{", start)
while True:
    depth += {"{": 1, "}": -1}.get(orig[i], 0)
    if depth == 0:
        break
    i += 1
end = i + 1
try:
    for n, body in enumerate(Path(variants).read_text().split("\n----\n")):
        src.write_text(orig[:start] + body.strip("\n") + orig[end:])
        r = subprocess.run(["ninja", f"build/src/{unit}.o"], capture_output=True, text=True)
        if r.returncode:
            print(f"[{n}] compile error: {r.stdout[-300:]}")
            continue
        out = subprocess.run(["python3", "tools/diff.py", unit, func], capture_output=True, text=True).stdout
        print(f"[{n}] {out.strip().splitlines()[-1]}")
finally:
    src.write_text(orig)
    subprocess.run(["ninja", f"build/src/{unit}.o"], capture_output=True)
