#!/usr/bin/env python3
"""Per-function match status of one unit from build/report.json (run `ninja report` first).
Usage: python3 tools/funcs.py <unit>     e.g. tools/funcs.py game/area"""
import json
import sys
from pathlib import Path

unit = sys.argv[1]
report = json.loads(Path("build/report.json").read_text())
for u in report["units"]:
    if u["name"] == unit:
        for f in u.get("functions", []):
            pct = f.get("fuzzy_match_percent")
            state = "asm" if pct is None else ("OK " if pct == 100 else f"{pct:5.1f}%")
            print(f"  {state:>6}  {f['name']:<28} {int(f.get('size', 0)):>5} bytes")
        break
else:
    sys.exit(f"unit {unit} not found")
