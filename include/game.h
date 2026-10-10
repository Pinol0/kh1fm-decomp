#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "libvu0.h"

/* General game flags (D_002C5958) */
typedef struct {
    /* bit 0  */ u32 unk_0 : 1;
    /* bit 1  */ u32 unk_1 : 1;
    /* bit 2  */ u32 unk_2 : 1;
    /* bit 3  */ u32 unk_3 : 10;
    /* bit 13 */ u32 unk_13 : 1;
    /* bit 14 */ u32 unk_14 : 3;
    /* bit 17 */ u32 unk_17 : 1;
    /* bit 18 */ u32 unk_18 : 1;
    /* bit 19 */ u32 unk_19 : 1;
    /* bit 20 */ u32 unk_20 : 7;
    /* bit 27 */ u32 unk_27 : 1;
    /* bit 28 */ u32 unk_28 : 4;
} GameFlags;

/* System flags (g_SystemFlags) */
typedef struct {
    /* bit 0 */ u32 unk_0 : 2;
    /* bit 2 */ u32 unk_2 : 1;
    /* bit 3 */ u32 unk_3 : 1;
    /* bit 4 */ u32 unk_4 : 1;
    /* bit 5 */ u32 unk_5 : 1;
    /* bit 6 */ u32 unk_6 : 26;
} SystemFlags;

/* More game flags (D_002C5960) */
typedef struct {
    /* bit 0 */ u32 unk_0 : 2;
    /* bit 2 */ u32 unk_2 : 1;
    /* bit 3 */ u32 unk_3 : 1; /* player input comes from func_001271B0 */
    /* bit 4 */ u32 unk_4 : 28;
} GameFlags2;

extern GameFlags D_002C5958;
extern GameFlags2 D_002C5960;
extern SystemFlags g_SystemFlags;
extern f32 D_002BBDFC; /* frame time step */

/* Ray cast against the map and the platforms (func_00118668) */
typedef struct {
    /* 0x00 */ sceVu0FVECTOR pos;  /* start */
    /* 0x10 */ sceVu0FVECTOR ray;  /* direction * length (y points down) */
    /* 0x20 */ sceVu0FVECTOR hit;  /* nearest hit */
    /* 0x30 */ s16 poly;           /* polygon hit, -1 if none */
    /* 0x32 */ u8 unk_32[2];
    /* 0x34 */ s32 platform;       /* platform hit, -1: the map */
    /* 0x38 */ u8 unk_38[8];
} CollisionRay; // size = 0x40

void func_00118668(CollisionRay* ray);
void func_00118628(CollisionRay* ray, s32 arg1);

#endif
