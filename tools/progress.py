#!/usr/bin/env python3
"""Print decompilation progress from build/report.json (run `ninja report` first)."""
import json
from pathlib import Path

report = json.loads(Path("build/report.json").read_text())


def line(name, m):
    code, total = int(m.get("matched_code", 0)), int(m.get("total_code", 0))
    funcs, total_funcs = m.get("matched_functions", 0), m.get("total_functions", 0)
    pct = 100 * code / total if total else 0
    print(f"{name:<10} {pct:6.2f}% code ({code}/{total} bytes)   {funcs}/{total_funcs} functions")


line("All", report["measures"])
for cat in report.get("categories", []):
    line(cat["name"], cat.get("measures", {}))
