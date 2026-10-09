#!/usr/bin/env python3
"""Minimal PINE client: read and write PS2 (EE) memory of a running PCSX2.

Enable PINE in PCSX2 (Settings > Advanced). Usage as a library:
    from pine import Pine
    p = Pine(); p.read32(0x2E26A0); p.readf(addr)
or from the command line:  python3 tools/pine.py info | r32 ADDR | rf ADDR [COUNT] | dump ADDR LEN
"""
import os
import socket
import struct
import sys

READ8, READ16, READ32, READ64, WRITE8, WRITE16, WRITE32, WRITE64 = range(8)
TITLE, GAME_ID, STATUS = 0xB, 0xC, 0xF


def default_socket():
    candidates = [
        os.path.join(os.environ.get("XDG_RUNTIME_DIR", "/tmp"), "pcsx2.sock"),
        f"/run/user/{os.getuid()}/.flatpak/net.pcsx2.PCSX2/xdg-run/pcsx2.sock",  # Flatpak
        "/tmp/pcsx2.sock",
    ]
    for c in candidates:
        if os.path.exists(c):
            return c
    raise FileNotFoundError("PINE socket not found: is PINE enabled and PCSX2 running?")


class Pine:
    def __init__(self, path=None):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(path or default_socket())

    def _call(self, opcode, args=b""):
        msg = struct.pack("<IB", 5 + len(args), opcode) + args
        self.sock.sendall(msg)
        head = self._recv(4)
        size = struct.unpack("<I", head)[0]
        body = self._recv(size - 4)
        if body[0] != 0:
            raise IOError(f"PINE error for opcode {opcode}")
        return body[1:]

    def _recv(self, n):
        data = b""
        while len(data) < n:
            chunk = self.sock.recv(n - len(data))
            if not chunk:
                raise IOError("PINE connection closed")
            data += chunk
        return data

    def read8(self, a): return self._call(READ8, struct.pack("<I", a))[0]
    def read32(self, a): return struct.unpack("<I", self._call(READ32, struct.pack("<I", a)))[0]
    def readf(self, a): return struct.unpack("<f", struct.pack("<I", self.read32(a)))[0]
    def write32(self, a, v): self._call(WRITE32, struct.pack("<II", a, v & 0xFFFFFFFF))
    def read(self, a, n):
        return b"".join(struct.pack("<I", self.read32(a + i)) for i in range(0, n, 4))[:n]

    def _string(self, op):
        data = self._call(op)
        n = struct.unpack("<I", data[:4])[0]
        return data[4:4 + n].rstrip(b"\0").decode("utf-8", "replace")

    def title(self): return self._string(TITLE)
    def game_id(self): return self._string(GAME_ID)
    def status(self): return ["running", "paused", "shutdown"][struct.unpack("<I", self._call(STATUS))[0]]


if __name__ == "__main__":
    p = Pine()
    cmd = sys.argv[1] if len(sys.argv) > 1 else "info"
    if cmd == "info":
        print(f"{p.game_id()} | {p.title()} | {p.status()}")
    elif cmd == "r32":
        a = int(sys.argv[2], 16); print(f"{a:08X}: {p.read32(a):08X}")
    elif cmd == "rf":
        a = int(sys.argv[2], 16); n = int(sys.argv[3]) if len(sys.argv) > 3 else 1
        print(" ".join(f"{p.readf(a + 4 * i):.3f}" for i in range(n)))
    elif cmd == "dump":
        a, n = int(sys.argv[2], 16), int(sys.argv[3], 16)
        d = p.read(a, n)
        for i in range(0, n, 16):
            row = d[i:i + 16]
            fl = " ".join(f"{struct.unpack('<f', row[j:j+4])[0]:10.3f}" for j in range(0, len(row), 4))
            print(f"{a+i:08X}  {row.hex(' ')}  |{fl}")
