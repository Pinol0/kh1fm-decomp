#ifndef GAME_H
#define GAME_H

#include "common.h"

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

extern GameFlags D_002C5958;
extern SystemFlags g_SystemFlags;
extern f32 D_002BBDFC; /* frame time step */

#endif
