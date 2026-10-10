# Area data formats

Tools: `tools/kingdom_img.py` reads files out of the ISO (KINGDOM.IDX / IMG),
`tools/collision2obj.py` exports the collision of an area `.bin` to OBJ (with a top-view PNG),
`tools/map2obj.py` exports its visible geometry (vertex colours, texture coordinates).

```sh
python3 tools/kingdom_img.py game.iso extract tw00_01.bin
python3 tools/collision2obj.py tw00_01.bin tw00_01.obj --png tw00_01.png
python3 tools/map2obj.py tw00_01.bin tw00_01_map.obj --png tw00_01_map.png
```

What the executable reads when an area loads. Verified against the game running in PCSX2
(Dive to the Heart) through PINE (`tools/pine.py`); offsets are from the start of each file.

## Names

`src/game/area_name.c` builds them from the world prefix (`dh`, `di`, `dc`, `tw`, ...) and the
area number: `tw.wdt` (world data), `tw03.ard` (area archive), `tw00_01` (base of the `.bin` /
`.img` pair).

## World data (`.wdt`)

Loaded at the fixed address `0x9A0000` (`g_WorldDataBuffer`), unless the world is unchanged.
Header: offsets from the file start (`WorldDataHeader` in `src/game/area.c`).

| Offset | Points to |
|---|---|
| 0x10 | area table: `{infoOffset, infoSize, entranceOffset, entranceSize}`, offsets from the table |
| 0x18, 0x20, 0x30, 0x38 | other tables (not identified yet) |

Area info records and entrance records are 0x40 bytes each (`AreaInfo`, `AreaEntrance`). An
entrance holds the area it leads into and the spawn place (x, y, z, angle) of the three party
members at +0x10, +0x20, +0x30.

## Area `.bin`

A container (`func_001012E0`): `u32 count` at 0x00, then `{u32 offset, u32 size}` pairs from 0x08.
Dive to the Heart has 9 entries.

| Entry (header offset) | Use |
|---|---|
| 1 (0x10) | collision set A (`func_00105288`) |
| 2 (0x18) | passed to `func_00106ED0` with entry 5: probably the map geometry (865 KB here) |
| 3 (0x20), 4 (0x28) | `func_00108CF8` with layer 0 and 1 |
| 5 (0x30) | used with entry 2 |
| 6-9 (0x38-0x50) | optional data stored in `D_0029B410`.. (present when count >= 6 / 8) |
| 10 (0x58) | collision set B (`func_00105D88`), when count >= 10 |

The matching `.img` (same base name) is loaded next; probably the textures.

## Collision

Header of offset/size pairs (`func_00105288`). Pointers in the two tables are relocated after
loading.

| Header | Data | Element size | Dive to the Heart |
|---|---|---|---|
| 0x08 / 0x0C | vertices (3 floats) | 12 | 1565 |
| 0x10 / 0x14 | ? | 8 | 77 |
| 0x18 / 0x1C | polygons | 20 (0x14) | 2283 |
| 0x20 / 0x24 | ? (planes or bounds) | 16 | 1 |
| 0x28 / 0x2C | pointers, relocated against 0x38 (-1 = none) | 4 | 1024 (acceleration grid) |
| 0x30 / 0x34 | node pointers, relocated against 0x40 | 4 | 191 |

A polygon (20 bytes): `u16` vertex indices at +0x00..+0x06 (the 4th is 0xFFFF or 0xFFFE for a
triangle), `u16` index into the 8-byte table at +0x08, attributes from +0x0A, flags `u32` at +0x10
(bit 0: ignored by the game).

A node is an axis-aligned box (min x, y, z at +0x00, max at +0x0C), a `u16` polygon count at
+0x18 and that many `u16` polygon indices from +0x1A. Polygon flags are at +0x10 (bit 0: ignored).

Queries (`func_00118378`) take a segment (start at +0x00, end at +0x10 of the query), gather the
candidate nodes from the grid, test the polygons of the nodes whose box meets the query box, and
return the nearest hit point (+0x20) and polygon (+0x30, -1 if none). Two collision sets can be
loaded; `func_00104528` / `func_00104678` switch the active one (`D_002A62EC`).

## Map geometry (`.bin` entry 2)

Read by `func_00106ED0`. `u32 parts` at 0x00, `u32` offset of a DMA table at 0x04, then one
0x80-byte record per part from 0x10: the 8 corners of its bounding box (vec4; some `w` hold
per-part parameters, reset to 1.0 on load). Table entry 0 is a header; entry `i + 1` is the DMA
"ref" tag of part `i` (`0x3000xxxx`, low 16 bits = quadword count) and the offset of its VIF
packet. Traverse Town's First District: 897 parts.

A VIF packet is a series of batches, each drawn by the VU1 program on `MSCNT`:

| VIF | Data |
|---|---|
| UNPACK V4-32, 1 qw @ 0 | GIF tag: vertex count, registers ST / RGBAQ / XYZ2 |
| STCYCL cl=3 wl=1 | vertices interleaved over 3 quadwords |
| UNPACK V4-32 @ 1 | position x, y, z, w; w = 0: no triangle, w != 0: closes a strip triangle (sign = winding) |
| UNPACK V4-8 (unsigned) @ 2 | colour RGBA (0x80 = 1.0): baked lighting |
| UNPACK V2-16 @ 3 | texture coordinates, 4096 = 1.0 |
| DIRECT, 3 qw | GS registers for the batch (texture) |

Not decoded yet: the textures (probably the `.img`, entries 3/4 and the DIRECT packets).
