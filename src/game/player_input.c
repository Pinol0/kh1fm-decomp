/*
 * Player input: turns the pad state into a move vector relative to the camera, and the
 * camera field of view it depends on.
 */
#include "common.h"
#include "game.h"
#include "actor.h"
#include "libc.h"

/* Processed pad state (D_002C5BB0). Partial. */
typedef struct {
    /* 0x00 */ u8 unk_00[8];
    /* 0x08 */ u32 unk_08;
    /* 0x0C */ u8 unk_0C[0x34];
    /* 0x40 */ f32 unk_40;
    /* 0x44 */ f32 unk_44;
    /* 0x48 */ u8 unk_48[4];
    /* 0x4C */ f32 strength;       /* left stick deflection */
    /* 0x50 */ u8 unk_50[0x10];
    /* 0x60 */ sceVu0FVECTOR stick; /* left stick direction */
} PadState;

/* Action requests from the buttons, cleared every frame */
typedef struct {
    /* 0x0 bit 0  */ u32 unk_0 : 8;
    /* 0x0 bit 8  */ u32 unk_8 : 16;
    /* 0x0 bit 24 */ u32 unk_24 : 1;
    /* 0x0 bit 25 */ u32 unk_25 : 7;
    /* 0x4 */ u8 unk_4[8];
} ActionRequest; // size = 0xC

typedef struct {
    /* 0x0 */ u8 unk_0[4];
    /* 0x4 */ f32 unk_4;
} Unk13DFB8;

extern ActionRequest D_002E3190;
extern s32 (*D_002E27D4)(Actor* actor, PadState* pad, sceVu0FVECTOR input); /* optional input filter */
extern PadState D_002C5BB0;
extern s32 D_002E27A8; /* control mode */
extern f32 D_004F77E0; /* extra yaw applied to the move direction */
extern sceVu0FVECTOR D_004F7800; /* last move input */
extern sceVu0FVECTOR D_002C58A0; /* zero */
void func_00126C10(Actor* actor, sceVu0FVECTOR input);
Unk13DFB8* func_0013DFB8(Actor* actor, sceVu0FVECTOR arg1);
void func_001586A8(s32 arg0);

f32 func_0026B8B8(f32 x); /* tanf */
f32 func_00120C88(f32 angle); /* wraps an angle to [-pi, pi] */
void func_00267810(sceVu0FMATRIX m0, sceVu0FMATRIX m1, f32 angle); /* rotation about y */
s32 func_00137618(void);
void func_00124E08(Actor* actor, PadState* pad, sceVu0FVECTOR input);
void func_00126F18(Actor* actor, PadState* pad, sceVu0FVECTOR input);

extern f32 D_002E27A0;  /* field of view (degrees) */
extern f32 D_004F77E4;  /* saved field of view */
extern f32 D_004F7650;  /* projection scale: 256 / tan(fov / 2) */
extern f32 D_002E27A4;  /* camera yaw */
extern sceVu0FMATRIX D_002C58D0; /* identity */

void func_00124C68(f32 fov) {
    D_002E27A0 = fov;
    D_004F7650 = 256.0f / func_0026B8B8(((fov * 0.5f) / 180.0f) * 3.1415928f);
}

/* Sets the field of view, saving the current one */
void func_00124CE0(f32 fov) {
    D_004F77E4 = D_002E27A0;
    func_00124C68(fov);
}

/* Restores the field of view saved by func_00124CE0 */
void func_00124D08(void) {
    func_00124C68(D_004F77E4);
}

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00124D28);

/* Turns a stick vector into a world direction: undoes the camera yaw, then applies angle */
void func_00124D58(sceVu0FVECTOR stick, sceVu0FVECTOR out, f32 angle, f32 strength) {
    sceVu0FMATRIX m;

    func_00267810(m, D_002C58D0, -func_00120C88(D_002E27A4));
    sceVu0ApplyMatrix(out, m, stick);
    out[0] = -out[0];
    out[3] = strength;
    func_00267810(m, D_002C58D0, angle);
    sceVu0ApplyMatrix(out, m, out);
}

#ifdef NON_MATCHING
// Equivalent; needs the original jump tables (still in the asm rodata) to match.
/* Move input from the left stick, depending on the control mode and the actor state */
void func_00124E08(Actor* actor, PadState* pad, sceVu0FVECTOR input) {
    sceVu0FMATRIX m;
    sceVu0FVECTOR stick;

    switch (D_002E27A8) {
        case 2:
            func_00267810(m, D_002C58D0, pad->unk_40 * 0.052359875f);
            sceVu0ApplyMatrix(input, m, actor->facing);
            input[1] = 0.0f;
            input[3] = 0.0f;
            return;
        case 11:
            if (pad->unk_44 > 0.0f) {
                if ((pad->unk_08 & 0x400000) && pad->unk_40 == 0.0f) {
                    sceVu0SubVector(input, D_002C58A0, actor->move);
                } else {
                    sceVu0CopyVector(input, actor->move);
                }
                input[1] = 0.0f;
                sceVu0Normalize(input, input);
                input[3] = 0.0f;
                return;
            }
            func_00267810(m, D_002C58D0, pad->unk_40 * 0.052359875f);
            sceVu0ApplyMatrix(input, m, actor->move);
            input[1] = 0.0f;
            sceVu0Normalize(input, input);
            input[3] = -pad->unk_44;
            return;
        default:
            sceVu0CopyVector(stick, pad->stick);
            switch (actor->state) {
                case 29:
                    sceVu0CopyVector(input, stick);
                    input[3] = pad->strength;
                    return;
                case 18:
                    stick[1] = 0.0f;
                case 10:
                case 13:
                case 15:
                    break;
                default:
                    /* stick (x, y) to the ground plane (x, 0, y) */
                    stick[2] = stick[1];
                    stick[1] = 0.0f;
                    break;
            }
            func_00124D58(stick, input, D_004F77E0, pad->strength);
            return;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00124E08);
#endif

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125000);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_001251A0);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125240);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_001252F8);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_001259B0);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125A00);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125A28);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125B68);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125BE8);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125CF0);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00125DC8);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126108);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126238);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126360);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126838);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_001269D8);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126BA0);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126C10);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126E80);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00126F18);

/* Move input of the player: stick to camera-relative vector, then button actions */
void func_001271B0(Actor* actor, sceVu0FVECTOR input) {
    sceVu0FVECTOR zero;
    Unk13DFB8* p;

    if (func_00137618() != 0) {
        func_00124E08(actor, &D_002C5BB0, input);
        sceVu0CopyVector(D_004F7800, input);
        if (D_002E27D4 != NULL) {
            if (D_002E27D4(actor, &D_002C5BB0, input) != 0) {
                func_00126F18(actor, &D_002C5BB0, input);
            }
        } else {
            func_00126F18(actor, &D_002C5BB0, input);
        }
    }
    if (D_002E3190.unk_24) {
        if ((actor->motion == 7 || actor->motion == 12) && (actor->state == 0 || actor->state == 12)) {
            actor->flags &= ~4;
            sceVu0CopyVector(zero, D_002C58A0);
            p = func_0013DFB8(actor, zero);
            if (p != NULL) {
                p->unk_4 = 2.0f;
                actor->unk_3B0 = 2.0f;
            }
        }
    } else if (D_002E3190.unk_0 != 0) {
        func_00126C10(actor, input);
    }
    memset(&D_002E3190, 0, sizeof(D_002E3190));
    if (actor->unk_14C != NULL && actor->unk_14C->unk_2C != NULL && (actor->unk_14C->unk_2C->unk_04 & 1)) {
        func_001586A8(11);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_00127340);

INCLUDE_ASM("asm/nonmatchings/game/player_input", func_001274C8);
