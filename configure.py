#!/usr/bin/env python3
"""Generate build.ninja for the Kingdom Hearts Final Mix (SLPS_251.98) decompilation.

Usage:
    ./configure.py          extract, split with splat and write build.ninja + objdiff.json
    ./configure.py --clean  remove everything generated first

Then `ninja` builds and checks the rom, `ninja report` writes build/report.json
(objdiff progress) and `python3 tools/progress.py` summarises it.
"""
import argparse
import hashlib
import json
import os
import stat
import shutil
import subprocess
import sys
import tarfile
import urllib.request
from pathlib import Path

import ninja_syntax
import splat.segtypes.common.asm as seg_asm
import splat.segtypes.common.bss as seg_bss
import splat.segtypes.common.c as seg_c
import splat.segtypes.common.data as seg_data
from splat.scripts import split

ROOT = Path(__file__).resolve().parent
BASENAME = "SLPS_251.98"
ELF = ROOT / BASENAME
ROM = ROOT / f"{BASENAME}.rom"
ELF_SHA1 = "e70bda789916142aafb53d85cef2e806b35ad8d8"
YAML = Path("config/kh1fm.yaml")
BUILD = Path("build")

CROSS = "mips-linux-gnu-"
CC_DIR = Path("tools/cc/ee-gcc2.96")
CC_URL = "https://github.com/decompme/compilers/releases/download/compilers/ee-gcc2.96.tar.xz"

OBJDIFF_VERSION = "v3.8.2"
OBJDIFF = Path("tools/objdiff-cli")
OBJDIFF_URL = f"https://github.com/encounter/objdiff/releases/download/{OBJDIFF_VERSION}/objdiff-cli-linux-x86_64"

AS_FLAGS = "-EL -march=r5900 -mabi=eabi -no-pad-sections -G0 -Iinclude"
CC_FLAGS = "-O2 -G0 -g -Iinclude -Isrc"


def check_elf():
    if not ELF.is_file():
        sys.exit(f"Missing {BASENAME}: copy it from your own disc of Kingdom Hearts Final Mix (JP).")
    if hashlib.sha1(ELF.read_bytes()).hexdigest() != ELF_SHA1:
        sys.exit(f"{BASENAME} has the wrong SHA-1; it must be the unmodified retail executable.")
    subprocess.run([f"{CROSS}objcopy", "-O", "binary", "--gap-fill=0x00", "-R", ".reginfo", str(ELF), str(ROM)], check=True)


def fetch_compiler():
    if (CC_DIR / "bin" / "ee-gcc").is_file():
        return
    print(f"Downloading ee-gcc 2.96 from {CC_URL}")
    CC_DIR.mkdir(parents=True, exist_ok=True)
    with urllib.request.urlopen(CC_URL) as response:
        with tarfile.open(fileobj=response, mode="r|xz") as archive:
            archive.extractall(CC_DIR)


def fetch_objdiff():
    if OBJDIFF.is_file():
        return
    print(f"Downloading objdiff-cli {OBJDIFF_VERSION}")
    urllib.request.urlretrieve(OBJDIFF_URL, OBJDIFF)
    OBJDIFF.chmod(OBJDIFF.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def category(name):
    return "sdk" if name.startswith("sdk/") else "game"


def clean():
    for path in ("asm", "build", "expected", "build.ninja", "objdiff.json", ".splache", ROM.name):
        p = ROOT / path
        if p.is_dir():
            shutil.rmtree(p)
        elif p.exists():
            p.unlink()


def write_ninja(entries):
    ninja = ninja_syntax.Writer(open(ROOT / "build.ninja", "w"), width=120)
    elf = BUILD / f"{BASENAME}.elf"
    rom = BUILD / f"{BASENAME}.rom"

    ninja.rule("as", description="as $in", command=f"{CROSS}as {AS_FLAGS} -o $out $in")
    ninja.rule("cc", description="cc $in", depfile="$out.d", deps="gcc",
               command=f"cpp -MM -MT $out -Iinclude -Isrc $in > $out.d && "
                       f"{CC_DIR}/bin/ee-gcc -c -B {CC_DIR}/bin/ee- {CC_FLAGS} -o $out $in && "
                       # rewrite the symbol table: ee-as 2.96 orders locals in a way modern ld rejects
                       f"{CROSS}strip $out -N dummy-symbol-name")
    ninja.rule("ld", description="link $out",
               command=f"{CROSS}ld -EL -T {BUILD}/undefined_funcs_auto.txt -T {BUILD}/undefined_syms_auto.txt "
                       f"-T linker_script_extra.ld -Map $mapfile -T $in -o $out")
    ninja.rule("rom", description="rom $out", command=f"{CROSS}objcopy -O binary --gap-fill=0x00 $in $out")
    ninja.rule("check", description="check $in", command=f"sha1sum -c $in && touch $out")
    ninja.rule("report", description="objdiff report", command=f"{OBJDIFF} report generate -o $out")

    objects = []
    units = []
    expected = []
    for entry in entries:
        seg = entry.segment
        if entry.object_path is None or seg.type.startswith("."):
            continue
        obj = str(entry.object_path)
        srcs = [str(s) for s in entry.src_paths]
        unit = {"name": seg.name, "metadata": {"progress_categories": [category(seg.name)]}}
        if isinstance(seg, seg_c.CommonSegC):
            ninja.build(obj, "cc", srcs)
            # objdiff target: splat's full disassembly of the same unit
            target = str(Path("expected") / f"{seg.name}.o")
            ninja.build(target, "as", str(seg.asm_out_path()))
            expected.append(target)
            unit.update(target_path=target, base_path=obj)
            unit["metadata"]["source_path"] = srcs[0]
        elif isinstance(seg, seg_asm.CommonSegAsm):
            ninja.build(obj, "as", srcs)
            unit["target_path"] = obj  # not decompiled yet: no base object
        elif isinstance(seg, (seg_data.CommonSegData, seg_bss.CommonSegBss)):
            ninja.build(obj, "as", srcs)
            unit = None
        else:
            sys.exit(f"Unsupported segment type {seg.type} ({seg.name})")
        objects.append(obj)
        if unit:
            units.append(unit)

    ninja.build(str(elf), "ld", str(BUILD / f"{BASENAME}.ld"), implicit=objects,
                variables={"mapfile": str(BUILD / f"{BASENAME}.map")})
    ninja.build(str(rom), "rom", str(elf))
    ninja.build(str(rom) + ".ok", "check", "config/checksum.sha1", implicit=[str(rom)])
    ninja.build(str(BUILD / "report.json"), "report", implicit=objects + expected + ["objdiff.json"])
    ninja.build("report", "phony", str(BUILD / "report.json"))
    ninja.default(str(rom) + ".ok")
    return units


def write_objdiff(units):
    config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "ninja",
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.c", "*.h", "*.s", "*.inc"],
        "progress_categories": [{"id": "game", "name": "Game"}, {"id": "sdk", "name": "Sony SDK"}],
        "units": units,
    }
    (ROOT / "objdiff.json").write_text(json.dumps(config, indent=2) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-c", "--clean", action="store_true", help="remove generated files first")
    args = parser.parse_args()

    if args.clean:
        clean()
    check_elf()
    fetch_compiler()
    fetch_objdiff()
    split.main([YAML], modes="all", verbose=False, use_cache=False, make_full_disasm_for_code=True)
    write_objdiff(write_ninja(split.linker_writer.entries))


if __name__ == "__main__":
    main()
