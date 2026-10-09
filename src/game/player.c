/*
 * Per-frame control of an actor: gathers the input move vector and runs the state
 * machine ("brain"), which then turns the actor toward its target angle.
 */
#include "common.h"
#include "game.h"
#include "actor.h"

void func_0013E1F8(Actor* actor, sceVu0FVECTOR input);
s32 func_001375D8(Actor* actor);
s32 func_0013F108(Actor* actor);
void func_001271B0(Actor* actor, sceVu0FVECTOR input);
void func_00159978(Actor* actor, sceVu0FVECTOR input);
void func_001707D0(Actor* actor, sceVu0FVECTOR input);
void func_0013E558(Actor* actor, sceVu0FVECTOR move);
s32 func_0014F600(Actor* arg0);
void func_0014CCE0(Actor* actor, f32 arg1);
void func_00130340(Actor* actor, s32 motion, f32 blend);
f32 func_001216D8(f32 angle, f32 target, f32 speed);
void func_00137688(Actor* actor, f32 angle);
void func_001452A0(Actor* actor);
extern u32 D_002C5AD4;
extern sceVu0FVECTOR D_002C58A0; /* zero */

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013E1F8);

/* Runs the update of the actor's state, then turns it toward its target angle */
void func_0013E558(Actor* actor, sceVu0FVECTOR move) {
    ActorStateFunc update = g_ActorStates[actor->state].update;

    if (update != NULL) {
        update(actor, move);
    }
    if (actor->state == 4) {
        sceVu0CopyVector(actor->unk_3C0, D_002C58A0);
        sceVu0CopyVector(actor->move, D_002C58A0);
    }
    func_00137688(actor, func_001216D8(actor->facing[3], actor->targetAngle, actor->params->turnSpeed));
    if ((actor->flags & 0x400) && actor->state == 2) {
        sceVu0ScaleVector(actor->unk_3C0, actor->move, actor->move[3]);
    }
    actor->flags &= ~0x800;
    func_001452A0(actor);
}

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013E638);

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013E978);

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013EAB0);

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013ECF8);

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013ED88);

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013EE40);

INCLUDE_ASM("asm/nonmatchings/game/player", func_0013F0C0);

s32 func_0013F108(Actor* actor) {
    if (actor->unk_398 != 0 && func_0014F600(actor->unk_398) != 0 && actor->state == 0) {
        func_0014CCE0(actor, 10.0f);
        sceVu0CopyVector(actor->unk_3C0, D_002C58A0);
        func_00130340(actor, 0x10006, 8.0f);
        return 1;
    }
    return 0;
}

/* Gathers the move input of an actor and runs its state machine */
void func_0013F198(Actor* actor) {
    sceVu0FVECTOR input;
    sceVu0FVECTOR dir;

    func_0013E1F8(actor, input);
    if (func_001375D8(actor) == 0 && func_0013F108(actor) == 0) {
        if (actor->unk_130->unk_54 == 0) {
            func_001271B0(actor, input);
        } else if (D_002C5958.unk_17) {
            func_00159978(actor, input);
        } else if (!D_002C5960.unk_3) {
            func_001707D0(actor, input);
        } else if (D_002C5AD4 & 0x8000) {
            func_001271B0(actor, input);
        }
    }
    /* Direction on the ground plane; keep the vertical part and the speed as they were */
    sceVu0CopyVector(dir, input);
    dir[1] = 0.0f;
    sceVu0Normalize(dir, dir);
    dir[1] = input[1];
    dir[3] = input[3];
    func_0013E558(actor, dir);
}
