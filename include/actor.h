#ifndef ACTOR_H
#define ACTOR_H

#include "common.h"
#include "libvu0.h"

/* Per-kind actor parameters (Actor.params) */
typedef struct {
    /* 0x00 */ u8 unk_00[4];
    /* 0x04 */ f32 unk_04;
    /* 0x08 */ f32 runThreshold; /* move speed from which the run motion is used */
    /* 0x0C */ u8 unk_0C[0xC0];
    /* 0xCC */ f32 turnSpeed;
} ActorParams;

/* Characters and objects of the field (Sora is *D_002E26A0[0]). Partial. */
typedef struct {
    /* 0x000 */ u32 flags;
    /* 0x004 */ u8 unk_004[0xC];
    /* 0x010 */ sceVu0FVECTOR pos;    /* y points down */
    /* 0x020 */ sceVu0FVECTOR move;   /* direction in xyz, speed in w */
    /* 0x030 */ sceVu0FVECTOR facing; /* w: current angle around y */
    /* 0x040 */ u8 unk_040[0x2C];
    /* 0x06C */ ActorParams* params;
    /* 0x070 */ s32 state;            /* index into the state handler table */
    /* 0x074 */ u8 unk_074[0x14];
    /* 0x088 */ u32 unk_088;
    /* 0x08C */ u8 unk_08C[0xC4];
    /* 0x150 */ f32 targetAngle;
    /* 0x154 */ u8 unk_154[0x10];
    /* 0x164 */ s32 motion;           /* current motion (animation) */
    /* 0x168 */ u8 unk_168[0x10];
    /* 0x178 */ s32 unk_178;
    /* 0x17C */ u8 unk_17C[0x50];
    /* 0x1CC */ u32 unk_1CC;
    /* 0x1D0 */ u8 unk_1D0[0x150];
    /* 0x320 */ sceVu0FVECTOR unk_320; /* saved position */
    /* 0x330 */ u8 unk_330[0x40];
    /* 0x370 */ u64 unk_370;           /* bit 50 follows D_002C5958.unk_0 */
    /* 0x378 */ u8 unk_378[0x26];
    /* 0x39E */ u8 unk_39E;
    /* 0x39F */ u8 unk_39F[0xA1];
    /* 0x440 */ u64 unk_440;           /* bit 33: the actor does not take move input */
} Actor;

#endif
