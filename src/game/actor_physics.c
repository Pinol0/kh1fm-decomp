/*
 * Actor physics: moves actors with collision against the map, in sub-steps no longer
 * than the size of their collision body, and tracks what is below them.
 */
#include "common.h"
#include "game.h"
#include "actor.h"

s32 func_001232F0(Actor* actor);
void func_00143A70(Actor* actor, sceVu0FVECTOR move, s32 arg2);
u8 func_00142618(Actor* actor);
s32 func_00123330(Actor* actor);
void func_0015BAD0(Actor* actor, s32 arg1, f32* out);
u8 func_00142388(sceVu0FVECTOR pos, u32* arg1);
void func_00143EB8(Actor* actor, sceVu0FVECTOR delta);
s32 func_00144060(Actor* actor);
s32 func_0014F658(Actor* actor, s32 arg1);
void func_0014CC88(Actor* actor, f32 arg1);
void func_00121FC4(sceVu0FVECTOR out, sceVu0FVECTOR v); /* direction in xyz, length in w */
extern sceVu0FVECTOR D_002C58B0; /* zero */
extern f32 D_002C5970;
f32 func_001427D0(Actor* actor, ActorBody* body);
Actor* func_0015C738(Actor* actor, sceVu0FVECTOR move);
void func_00137440(Actor* actor, Actor* platform);
void func_00143818(Actor* actor, sceVu0FVECTOR move, s32 arg2);
void func_00143180(Actor* actor, sceVu0FVECTOR move);
void func_0014F798(Actor* actor, sceVu0FVECTOR delta);
void func_001568E8(Actor* actor, sceVu0FVECTOR out);
void func_001564E0(Actor* actor, sceVu0FVECTOR delta);
s32 func_0014F5C8(void);
s32 func_0014F600(Actor* actor);
void func_0012C698(Actor* actor);
s32 func_001443A8(Actor* actor);
u64 func_001166E8(sceVu0FVECTOR pos); /* bit i: overlapping platform i - 1 (bit 0: the map) */
void func_00142FE0(Actor* actor, Actor* other);
s32 func_00143080(Actor* actor, sceVu0FVECTOR move);
void func_0014D070(Actor* actor, sceVu0FVECTOR move);
f32 func_00121748(sceVu0FVECTOR a, sceVu0FVECTOR b);
f32 func_00120BF8(sceVu0FVECTOR v);
extern sceVu0FVECTOR D_002C58A0; /* zero */
void func_00151B98(Actor* actor, sceVu0FVECTOR move);
void func_001337E0(s32 arg0, Actor* actor, s32 surface);
void func_00117710(ActorBody* body, sceVu0FVECTOR arg1);
extern Actor* D_002E26A0[3]; /* party: 0 = Sora */

typedef struct {
    /* 0x0 */ Actor* actor;
    /* 0x4 */ u8 unk_4[0xC];
} Platform; // size = 0x10

extern Platform D_003044E0[];

typedef struct {
    /* 0x0 */ u8 unk_0[8];
    /* 0x8 */ Platform entries[1]; /* = D_003044E0 */
} PlatformTable;

extern PlatformTable D_003044D8;

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142388);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142470);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142480);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_001425D0);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142618);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_001426A8);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142730);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_001427D0);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_001429F8);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142B30);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142C80);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142CF0);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142E30);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00142FE0);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00143080);

/*
 * Resolves one collision step: tries the move from the current body position, then applies
 * the per-state limits (states 10 and 18 refuse horizontal moves that the collision blocked)
 * and turns the result back into the move that is really made.
 */
void func_00143180(Actor* actor, sceVu0FVECTOR move) {
    sceVu0FVECTOR delta;
    ActorBody saved;
    ActorBody test;
    u32 flags;
    s32 count;
    s32 i;
    Platform* platform;

    if ((actor->unk_130->unk_04 & 0x2000) && ((u32)(actor->unk_370 >> 53) & 1) && actor->unk_398 == NULL
        && move[0] == 0.0f && move[1] == 0.0f && move[2] == 0.0f) {
        return;
    }
    flags = actor->body.flags;
    while (1) {
        sceVu0CopyVector(actor->body.vel, move);
        actor->body.vel[3] = 1.0f;
        saved = actor->body;
        sceVu0CopyVector(saved.target, actor->body.pos);
        if (actor->flags & 0x100) {
            if (func_00143080(actor, move) == 0) {
                actor->body = saved;
                sceVu0CopyVector(actor->body.target, actor->body.pos);
            }
        } else {
            count = 0;
            actor->unk_090 = func_001166E8(actor->body.pos);
            platform = (Platform*)((u8*)&D_003044D8 - 8); // TODO fake match: &D_003044D8.entries[-1] folds the -8
            for (i = 0; i < 32; i++, platform++) {
                if ((s32)((actor->unk_090 >> i) & 1)) {
                    if (i > 0) {
                        func_00142FE0(actor, platform->actor);
                        if ((u32)(platform->actor->unk_370 >> 57) & 1) {
                            count++;
                        }
                    } else {
                        count++;
                    }
                }
            }
            if (count >= 2) {
                test = actor->body;
                sceVu0CopyVector(test.pos, test.target);
                sceVu0CopyVector(test.vel, D_002C58A0);
                if (func_001166E8(test.pos) != 0) {
                    actor->body = saved;
                    actor->body.unk_28 |= 0x80000000;
                }
            }
        }
        switch (actor->state) {
        case 2:
        case 3:
            func_0014D070(actor, move);
            break;
        case 10:
            sceVu0SubVector(actor->body.vel, actor->body.target, actor->body.pos);
            if (move[0] != 0.0f || move[2] != 0.0f) {
                if (actor->body.unk_3E == 0xFFFF || func_00121748(actor->body.vel, move) < 1.0f) {
                    move[2] = 0.0f;
                    move[0] = 0.0f;
                    continue;
                }
            } else if (actor->body.unk_3E == 0xFFFF) {
                sceVu0CopyVector(move, D_002C58A0);
                sceVu0CopyVector(actor->body.target, actor->body.pos);
                actor->body.unk_3E = 0;
            }
            break;
        case 15:
            sceVu0SubVector(delta, actor->body.target, actor->body.pos);
            if (func_00120BF8(delta) > 15.0f) {
                actor->unk_148->unk_90 = 0xFFFF;
            }
            sceVu0CopyVector(actor->body.target, actor->body.pos);
            break;
        case 18:
            if (actor->body.unk_6E == 0xFFFF) {
                if (actor->body.flags & 0x20) {
                    actor->body.unk_6E = 0;
                } else if (!(flags & 0x20)) {
                    if (move[0] == 0.0f && move[2] == 0.0f) {
                        sceVu0CopyVector(actor->body.target, actor->body.pos);
                        actor->body.unk_6E = 0;
                    } else {
                        move[0] = move[2] = 0.0f;
                        continue;
                    }
                }
            }
            break;
        }
        break;
    }
    sceVu0SubVector(move, actor->body.target, actor->body.pos);
    actor->unk_370 |= (u64)0x8000 << 38; /* bit 53 */
    actor->unk_370 |= (u64)0x8000 << 41; /* bit 56 */
}

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00143660);

INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00143818);

#ifdef NON_MATCHING
// Equivalent; two instructions short (register use around the surface tracking).
/*
 * Moves an actor with collision: the move is split into steps no longer than the
 * collision step size (at most 4), each one resolved against the map.
 */
void func_00143A70(Actor* actor, sceVu0FVECTOR move, s32 arg2) {
    sceVu0FVECTOR dir;
    f32 size;
    f32 step;
    u32 flags;
    u16 surface;
    Actor* platform;
    s32 i;

    size = func_001427D0(actor, &actor->body);
    step = (size < actor->unk_4AC) ? size : actor->unk_4AC;
    actor->unk_4AC = size;
    if (actor->unk_398 != NULL) {
        actor->unk_370 &= ~((u64)1 << 53);
    }
    func_00137440(actor, func_0015C738(actor, move));
    func_00143818(actor, move, arg2);
    func_00121FC4(dir, move);
    if (dir[3] / step > 4.0f) {
        dir[3] = step * 4.0f;
    }
    surface = 0;
    flags = 1;
    do {
        if (step < dir[3]) {
            sceVu0ScaleVector(move, dir, step);
            dir[3] -= step;
        } else {
            sceVu0ScaleVector(move, dir, dir[3]);
            dir[3] = 0.0f;
        }
        func_001427D0(actor, &actor->body);
        func_00143180(actor, move);
        flags = (flags | (actor->body.flags & ~1)) & ((actor->body.flags & 1) | ~1);
        if (actor->unk_130->unk_4C == 0xC9) {
            func_00151B98(actor, move);
        }
        sceVu0AddVector(actor->pos, actor->pos, move);
        if (actor->body.surface != 0 && actor->body.surface != surface) {
            if (actor->body.flags & 4) {
                func_001337E0(-2, actor, actor->body.surface);
                surface = actor->body.surface;
            } else if (actor == D_002E26A0[0]) {
                func_001337E0(-2, actor, actor->body.surface);
                surface = actor->body.surface;
            } else {
                surface = actor->body.surface;
            }
        }
        actor->unk_448 = actor->body.surface;
    } while (dir[3] > 0.0f);
    actor->body.flags = flags;
    func_00117710(&actor->body, actor->unk_320);
    sceVu0Normalize(actor->unk_0C0, actor->unk_340);
    actor->unk_0C0[3] = 1.0f;
    platform = actor->unk_398;
    if (actor->body.platform != -1) {
        platform = D_003044E0[actor->body.platform].actor;
        if (!(actor->body.flags & 1)) {
            func_00137440(actor, platform);
        }
    }
    if (platform != NULL) {
        for (i = 0; i < 4; i++) {
            actor->unk_350[i] = platform->unk_350[i];
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00143A70);
#endif

#ifdef NON_MATCHING
// Equivalent; the original reloads unk_3A0 inside the copy loop.
/* Moves an actor by `move`, with collision when it uses physics */
void func_00143D20(Actor* actor, sceVu0FVECTOR move, s32 arg2) {
    sceVu0FVECTOR delta;
    sceVu0FVECTOR prev;
    f32 box[8];
    s32 i;

    sceVu0CopyVector(delta, move);
    sceVu0CopyVector(prev, actor->pos);
    if (func_001232F0(actor) != 0) {
        func_00143A70(actor, delta, arg2);
        actor->unk_39E = func_00142618(actor);
        if (actor->unk_398 != NULL && func_00123330(actor->unk_398) != 0) {
            func_0015BAD0(actor->unk_398, actor->unk_398->unk_134->unk_14, box);
            actor->unk_3D4 = box[5] - box[2] - actor->pos[1];
        } else {
            actor->unk_3D4 = actor->unk_320[1] - actor->pos[1];
        }
    } else {
        sceVu0AddVector(actor->pos, actor->pos, delta);
        actor->unk_3D4 = 0.0f;
        if ((u32)(actor->kind - 2) < 2) {
            sceVu0CopyVector(prev, actor->unk_050);
            prev[1] -= 100.0f;
            actor->unk_39E = func_00142388(prev, actor->unk_350);
        } else if (actor->unk_3A0 != NULL && actor->unk_3A0->kind == 3) {
            for (i = 0; i < 4; i++) {
                actor->unk_350[i] = actor->unk_3A0->unk_350[i];
            }
        }
    }
    sceVu0SubVector(delta, actor->pos, prev);
}
#else
INCLUDE_ASM("asm/nonmatchings/game/actor_physics", func_00143D20);
#endif

/* The move an actor makes this frame: from its state, plus the pushes it is subject to. */
void func_00143EB8(Actor* actor, sceVu0FVECTOR delta) {
    sceVu0FVECTOR extra;
    ActorStateFunc move;

    switch (actor->state) {
    case 9:
    case 10:
    case 22:
    case 23:
        sceVu0ScaleVectorXYZ(delta, actor->move, actor->move[3] * D_002BBDFC);
        break;
    case 18:
        sceVu0ScaleVectorXYZ(delta, actor->move, D_002BBDFC);
        break;
    case 20:
        sceVu0CopyVector(delta, actor->move);
        break;
    case 19:
        sceVu0CopyVector(delta, D_002C58A0);
        break;
    default:
        move = g_ActorStates[actor->state].move;
        if (move != NULL) {
            move(actor, delta);
        } else {
            func_0014F798(actor, delta);
        }
        break;
    }
    if ((g_ActorStates[actor->state].flags >> 3) & 1) {
        func_001568E8(actor, extra);
        sceVu0AddVector(delta, delta, extra);
    }
    if (actor->unk_088 & 0x40) {
        func_001564E0(actor, delta);
    }
    if ((s32)(*(u64*)&actor->unk_160 >> 2) & 1) {
        sceVu0AddVector(delta, delta, actor->unk_200);
    }
    sceVu0AddVector(delta, delta, actor->unk_120);
}

/* The state an actor switches to after moving (0 or 2 unless the state decides). */
s32 func_00144060(Actor* actor) {
    s32 grounded;
    s32 (*next)(Actor*, s32);

    if (actor->pos[1] >= actor->body.unk_34) {
        if ((func_0014F5C8() != 0 || func_0014F600(actor) != 0) && !(actor->flags & 0x400000)) {
            func_0012C698(actor);
        }
        actor->flags |= 0x400000;
    } else {
        actor->flags &= ~0x400000;
    }
    grounded = 0;
    if (actor->body.flags & 1) {
        grounded = actor->unk_398 == NULL;
    }
    switch (actor->state) {
    case 11:
    case 19:
    case 20:
    case 22:
    case 23:
        return actor->state;
    case 16:
        if (grounded) {
            return actor->state;
        }
        break;
    case 17:
        if (func_001443A8(actor) != 0) {
            return actor->state;
        }
        if (grounded) {
            return actor->state;
        }
        break;
    default:
        next = g_ActorStates[actor->state].next;
        if (next != NULL) {
            return next(actor, grounded);
        }
        break;
    }
    if (grounded || (actor->flags & 0x1000)) {
        return 2;
    }
    return 0;
}

/* Per-frame movement of an actor: walking along its move vector, or physics */
void func_001441B8(Actor* actor) {
    sceVu0FVECTOR delta;
    s32 ground;

    sceVu0CopyVector(actor->unk_490, actor->pos);
    actor->unk_090 = 0;
    if (func_001232F0(actor) == 0) {
        sceVu0CopyVector(actor->unk_3C0, D_002C58B0);
        sceVu0ScaleVector(delta, actor->move, actor->move[3] * D_002BBDFC);
        if ((s32)(*(u64*)&actor->unk_160 >> 2) & 1) {
            sceVu0AddVector(delta, delta, actor->unk_200);
        }
        sceVu0AddVector(delta, delta, actor->unk_120);
        func_00143D20(actor, delta, 0);
    } else {
        func_00143EB8(actor, delta);
        func_00143D20(actor, delta, 0);
        ground = func_00144060(actor);
        if (func_0014F658(actor, ground) != 0 && ground == 2) {
            func_0014CC88(actor, 0.0f);
            actor->flags &= ~4;
            sceVu0ScaleVector(delta, delta, D_002C5970);
            func_00121FC4(delta, delta);
            if (delta[3] > 8.0f) {
                delta[3] = 8.0f;
            }
            sceVu0ScaleVector(actor->unk_3C0, delta, delta[3]);
        }
    }
    sceVu0CopyVector(actor->unk_120, D_002C58B0);
}
