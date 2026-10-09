# Kingdom Hearts Final Mix — decompilation

A matching decompilation of the main executable of **Kingdom Hearts Final Mix** for PlayStation 2
(Japan, `SLPS_251.98`). The long-term goal is a native port, starting with the PlayStation Vita.

This repository contains no game code or assets. You need your own copy of the game.

## Setup

1. Copy `SLPS_251.98` from your disc into the repository root
   (SHA-1 `e70bda789916142aafb53d85cef2e806b35ad8d8`).
2. Build the container (or install the same tools locally):
   `podman build -t kh1fm -f docker/Containerfile .`
3. Configure and build:
   `podman run --rm -v "$PWD":/work:Z kh1fm sh -c "./configure.py && ninja"`

A successful build ends with `build/SLPS_251.98.rom: OK`.

The ee-gcc 2.96 compiler is downloaded on first configure from the
[decomp.me compiler collection](https://github.com/decompme/compilers).

## License

[CC0 1.0](LICENSE). This covers the work in this repository only; the game itself remains the
property of its owners.
