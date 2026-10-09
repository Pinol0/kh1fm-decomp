#!/usr/bin/env python3
"""Generate build.ninja for the Kingdom Hearts Final Mix (SLPS_251.98) decompilation.

Usage:
    ./configure.py          extract, split with splat and write build.ninja
    ./configure.py --clean  remove everything generated first
"""
import argparse
import hashlib
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


def clean():
    for path in ("asm", "build", "expected", "build.ninja", ".splache", ROM.name):
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
               command=f"{CC_DIR}/bin/ee-gcc -c -B {CC_DIR}/bin/ee- {CC_FLAGS} -MMD -MF $out.d -o $out $in")
    ninja.rule("ld", description="link $out",
               command=f"{CROSS}ld -EL -T {BUILD}/undefined_funcs_auto.txt -T {BUILD}/undefined_syms_auto.txt "
                       f"-T linker_script_extra.ld -Map $mapfile -T $in -o $out")
    ninja.rule("rom", description="rom $out", command=f"{CROSS}objcopy -O binary --gap-fill=0x00 $in $out")
    ninja.rule("check", description="check $in", command=f"sha1sum -c $in && touch $out")

    objects = []
    for entry in entries:
        seg = entry.segment
        if entry.object_path is None or seg.type.startswith("."):
            continue
        obj = str(entry.object_path)
        srcs = [str(s) for s in entry.src_paths]
        if isinstance(seg, seg_c.CommonSegC):
            ninja.build(obj, "cc", srcs)
        elif isinstance(seg, (seg_asm.CommonSegAsm, seg_data.CommonSegData, seg_bss.CommonSegBss)):
            ninja.build(obj, "as", srcs)
        else:
            sys.exit(f"Unsupported segment type {seg.type} ({seg.name})")
        objects.append(obj)

    ninja.build(str(elf), "ld", str(BUILD / f"{BASENAME}.ld"), implicit=objects,
                variables={"mapfile": str(BUILD / f"{BASENAME}.map")})
    ninja.build(str(rom), "rom", str(elf))
    ninja.build(str(rom) + ".ok", "check", "config/checksum.sha1", implicit=[str(rom)])
    ninja.default(str(rom) + ".ok")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-c", "--clean", action="store_true", help="remove generated files first")
    args = parser.parse_args()

    if args.clean:
        clean()
    check_elf()
    fetch_compiler()
    split.main([YAML], modes="all", verbose=False)
    write_ninja(split.linker_writer.entries)


if __name__ == "__main__":
    main()
