# Area data formats

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

A node is an axis-aligned box (min x, y, z at +0x00, max at +0x0C), a `u16` polygon count at
+0x18 and that many `u16` polygon indices from +0x1A. Polygon flags are at +0x10 (bit 0: ignored).

Queries (`func_00118378`) take a segment (start at +0x00, end at +0x10 of the query), gather the
candidate nodes from the grid, test the polygons of the nodes whose box meets the query box, and
return the nearest hit point (+0x20) and polygon (+0x30, -1 if none). Two collision sets can be
loaded; `func_00104528` / `func_00104678` switch the active one (`D_002A62EC`).
