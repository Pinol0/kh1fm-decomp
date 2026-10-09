# Kingdom Hearts Final Mix — decompilation

A matching decompilation of the main executable of **Kingdom Hearts Final Mix** for PlayStation 2
(Japan, `SLPS_251.98`): C code that compiles, with the original compiler, into the exact bytes of
the retail game.

The long-term goal is a **native port**, starting with the PlayStation Vita, so modules needed to
load and play an area (area data, player, camera, collision, rendering) come first.

This repository contains no game code or assets. You need your own copy of the game.

## Progress

`ninja report && python3 tools/progress.py` prints the current state, split between game code and the
Sony SDK libraries (which a port replaces, so they are tracked separately).

## Setup

1. Copy `SLPS_251.98` from your disc into the repository root
   (SHA-1 `e70bda789916142aafb53d85cef2e806b35ad8d8`).
2. Build the container (or install the same tools locally):
   `podman build -t kh1fm -f docker/Containerfile .`
3. Configure and build:
   `podman run --rm -v "$PWD":/work:Z kh1fm sh -c "./configure.py && ninja"`

A successful build ends with `build/SLPS_251.98.rom: OK`.

The ee-gcc 2.96 compiler and objdiff-cli are downloaded on first configure
(from the [decomp.me compiler collection](https://github.com/decompme/compilers) and
[objdiff](https://github.com/encounter/objdiff)).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow, tools and conventions.

## Related work

- [ethteck/kh1](https://github.com/ethteck/kh1): decompilation of the original Japanese release
  and Final Mix, a separate project.
- [OpenKH](https://github.com/OpenKH/OpenKh): documentation and tools for the game's file formats.

## License

[CC0 1.0](LICENSE). This covers the work in this repository only; the game itself remains the
property of its owners.
