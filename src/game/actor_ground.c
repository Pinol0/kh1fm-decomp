/*
 * State 0 of the actor state machine: on the ground. The update runs the action checks
 * and, when none takes over, moves the actor along the input vector.
 */
#include "common.h"
#include "game.h"
#include "actor.h"

s32 func_001404D8(Actor* actor, sceVu0FVECTOR move);
s32 func_001401D8(Actor* actor, sceVu0FVECTOR move);
s32 func_0013F510(Actor* actor, sceVu0FVECTOR move);
s32 func_0015D770(Actor* actor, sceVu0FVECTOR move);
s32 func_00144410(Actor* actor, sceVu0FVECTOR move);
s32 func_0015DD50(Actor* actor, sceVu0FVECTOR move);
void func_0014F658(Actor* actor, s32 arg1);
void func_0015AC58(void);
s32 func_0013DB30(void);
void func_0014CB90(Actor* actor);
void func_001376E8(Actor* actor, sceVu0FVECTOR move);
s32 func_00131B08(Actor* actor);
s32 func_00130010(Actor* actor);
void func_00130398(Actor* actor, s32 motion, f32 blend);
void func_00146C98(Actor* actor, s32 arg1);
void func_001315C0(Actor* actor, s32 id, sceVu0FVECTOR out, f32 scale);
void func_0012BD98(Actor* actor, s32 arg1, u32 arg2, sceVu0FVECTOR pos);
extern s32 D_002C5C54;

/*
 * Applies an input move vector (direction in xyz, speed in w) and picks the motion:
 * base = standing, base + 1 = walking, base + 2 = running.
 */
#ifdef NON_MATCHING
// Equivalent; `next` does not get its own saved register (one instruction short).
void func_00159F78(Actor* actor, sceVu0FVECTOR move, s32 base) {
    sceVu0FVECTOR point;
    s32 next;

    if (move[0] != 0.0f || move[1] != 0.0f || move[2] != 0.0f) {
        func_001376E8(actor, move);
        sceVu0CopyVector(actor->move, move);
        if (!(actor->flags & 4) && !((actor->unk_440 >> 33) & 1)) {
            if (move[3] == 0.0f) {
                if (!(actor->unk_1CC & 0x100000)) {
                    if (((actor->unk_370 >> 50) & 1) != D_002C5958.unk_0 &&
                        (D_002C5958.unk_0 != 0 || (actor->motion == 0 && func_00131B08(actor) != 0))) {
                        actor->unk_370 = (actor->unk_370 & ~((u64)1 << 50)) | ((u64)D_002C5958.unk_0 << 50);
                        func_00130398(actor, base, 16.0f);
                    } else if (actor->unk_178 == 0) {
                        if ((actor->motion == base + 1 || actor->motion == base + 2) && D_002C5C54 == 0) {
                            func_00146C98(actor, 8);
                        }
                        func_00130398(actor, base, 10.0f);
                    }
                }
            } else {
                actor->unk_370 = (actor->unk_370 & ~((u64)1 << 50)) | ((u64)D_002C5958.unk_0 << 50);
                if (actor->unk_088 & 0x80) {
                    actor->move[3] = move[3] = actor->params->unk_04;
                }
                next = base + 1;
                if (!(move[3] < actor->params->runThreshold)) {
                    next = base + 2;
                }
                if (actor->motion == base && func_00130010(actor) == 0) {
                    func_001315C0(actor, 0x29, point, 1.0f);
                    func_0012BD98(actor, actor->unk_39E, 0x80000002, point);
                    func_001315C0(actor, 0x28, point, 1.0f);
                    func_0012BD98(actor, actor->unk_39E, 0x80000002, point);
                }
                func_00130398(actor, next, 8.0f);
            }
        }
    } else {
        sceVu0CopyVector(actor->move, actor->facing);
        actor->move[3] = 0.0f;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/game/actor_ground", func_00159F78);
#endif

/* Update of the ground state */
void func_0015A258(Actor* actor, sceVu0FVECTOR move) {
    if (func_001404D8(actor, move) == 0 && func_001401D8(actor, move) == 0 &&
        func_0013F510(actor, move) == 0 && func_0015D770(actor, move) == 0 &&
        func_00144410(actor, move) == 0 && func_0015DD50(actor, move) == 0) {
        if (D_002C5958.unk_17) {
            func_0014F658(actor, 30);
        } else {
            func_00159F78(actor, move, 0);
        }
    }
}

/* Next state when the ground state ends */
#ifdef NON_MATCHING
// Equivalent; the original tests flags & 0x100 separately on each path.
s32 func_0015A348(Actor* actor, s32 arg1) {
    if (arg1 != 0 || (actor->flags & 0x1000)) {
        if (actor->flags & 0x100) {
            sceVu0CopyVector(actor->pos, actor->unk_320);
        } else {
            if (actor->motion == 0xDC) {
                func_0015AC58();
            }
            return 2;
        }
    }
    return actor->state;
}
#else
INCLUDE_ASM("asm/nonmatchings/game/actor_ground", func_0015A348);
#endif

void func_0015A3D8(Actor* actor) {
    if (func_0013DB30() != 0) {
        func_0014CB90(actor);
    }
}
