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

struct ActorUnk130 {
    /* 0x00 */ u8 unk_00[0x4C];
    /* 0x4C */ u16 unk_4C;
    /* 0x4E */ u8 unk_4E[6];
    /* 0x54 */ s8 unk_54; /* 0: input comes from func_001271B0 */
};

struct ActorUnk134 {
    /* 0x00 */ u8 unk_00[0x14];
    /* 0x14 */ s32 unk_14;
};

struct ActorUnk14C_2C {
    /* 0x00 */ u8 unk_00[4];
    /* 0x04 */ u32 unk_04;
};

struct ActorUnk14C {
    /* 0x00 */ u8 unk_00[0x2C];
    /* 0x2C */ struct ActorUnk14C_2C* unk_2C;
};

/* Collision body of an actor (Actor.body) */
typedef struct {
    /* 0x00 */ sceVu0FVECTOR pos;
    /* 0x10 */ sceVu0FVECTOR vel;
    /* 0x20 */ f32 radius;
    /* 0x24 */ u8 unk_24[8];
    /* 0x2C */ s32 platform; /* index into the platform table, -1 if none */
    /* 0x30 */ u32 flags;
    /* 0x34 */ f32 unk_34;
    /* 0x38 */ u16 surface;  /* surface type below, 0 if none */
    /* 0x3A */ u8 unk_3A[6];
} ActorBody; // size = 0x40

typedef struct Actor Actor;
typedef void (*ActorStateFunc)(Actor* actor, sceVu0FVECTOR move);

/* One row per actor state: g_ActorStates[actor->state] */
typedef struct {
    /* 0x00 */ void* unk_00;
    /* 0x04 */ ActorStateFunc update;
    /* 0x08 */ void* unk_08[6];
} ActorState; // size = 0x20

/* Characters and objects of the field (Sora is *D_002E26A0[0]). Partial. */
struct Actor {
    /* 0x000 */ u32 flags;
    /* 0x004 */ u8 unk_004[2];
    /* 0x006 */ u8 kind;
    /* 0x007 */ u8 unk_007[9];
    /* 0x010 */ sceVu0FVECTOR pos;    /* y points down */
    /* 0x020 */ sceVu0FVECTOR move;   /* direction in xyz, speed in w */
    /* 0x030 */ sceVu0FVECTOR facing; /* w: current angle around y */
    /* 0x040 */ u8 unk_040[0x10];
    /* 0x050 */ sceVu0FVECTOR unk_050;
    /* 0x060 */ u8 unk_060[0xC];
    /* 0x06C */ ActorParams* params;
    /* 0x070 */ s32 state;            /* index into the state handler table */
    /* 0x074 */ u8 unk_074[0x14];
    /* 0x088 */ u32 unk_088;
    /* 0x08C */ u8 unk_08C[4];
    /* 0x090 */ u64 unk_090;
    /* 0x098 */ u8 unk_098[0x28];
    /* 0x0C0 */ sceVu0FVECTOR unk_0C0; /* normalized ground normal */
    /* 0x0D0 */ u8 unk_0D0[0x50];
    /* 0x120 */ sceVu0FVECTOR unk_120; /* impulse, cleared every frame */
    /* 0x130 */ struct ActorUnk130* unk_130;
    /* 0x134 */ struct ActorUnk134* unk_134;
    /* 0x138 */ u8 unk_138[0x14];
    /* 0x14C */ struct ActorUnk14C* unk_14C;
    /* 0x150 */ f32 targetAngle;
    /* 0x154 */ u8 unk_154[0xC];
    /* 0x160 */ u32 unk_160;          /* read as a u64 together with motion */
    /* 0x164 */ s32 motion;           /* current motion (animation) */
    /* 0x168 */ u8 unk_168[0x10];
    /* 0x178 */ s32 unk_178;
    /* 0x17C */ u8 unk_17C[0x50];
    /* 0x1CC */ u32 unk_1CC;
    /* 0x1D0 */ u8 unk_1D0[0x30];
    /* 0x200 */ sceVu0FVECTOR unk_200; /* push from what the actor stands on */
    /* 0x210 */ u8 unk_210[0x90];
    /* 0x2A0 */ ActorBody body;
    /* 0x2E0 */ u8 unk_2E0[0x40];
    /* 0x320 */ sceVu0FVECTOR unk_320; /* saved position */
    /* 0x330 */ u8 unk_330[0x10];
    /* 0x340 */ sceVu0FVECTOR unk_340; /* ground normal */
    /* 0x350 */ u32 unk_350[4] __attribute__((aligned(16)));
    /* 0x360 */ u8 unk_360[0x10];
    /* 0x370 */ u64 unk_370;           /* bit 50 follows D_002C5958.unk_0 */
    /* 0x378 */ u8 unk_378[0x20];
    /* 0x398 */ struct Actor* unk_398; /* actor this one stands on */
    /* 0x39C */ u8 unk_39C[2];
    /* 0x39E */ u8 unk_39E;
    /* 0x39F */ u8 unk_39F[1];
    /* 0x3A0 */ struct Actor* unk_3A0;
    /* 0x3A4 */ u8 unk_3A4[0xC];
    /* 0x3B0 */ f32 unk_3B0;
    /* 0x3B4 */ u8 unk_3B4[0xC];
    /* 0x3C0 */ sceVu0FVECTOR unk_3C0; /* velocity used by the physics */
    /* 0x3D0 */ u8 unk_3D0[4];
    /* 0x3D4 */ f32 unk_3D4;           /* height above what is below */
    /* 0x3D8 */ u8 unk_3D8[0x68];
    /* 0x440 */ u64 unk_440;           /* bit 33: the actor does not take move input */
    /* 0x448 */ s32 unk_448;           /* last surface stood on */
    /* 0x44C */ u8 unk_44C[0x44];
    /* 0x490 */ sceVu0FVECTOR unk_490; /* position at the start of the frame */
    /* 0x4A0 */ u8 unk_4A0[0xC];
    /* 0x4AC */ f32 unk_4AC;           /* previous collision step */
};

extern ActorState g_ActorStates[];

#endif
