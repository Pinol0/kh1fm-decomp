/*
 * Area management: per-area info table, loading state and related flags.
 */
#include "common.h"

typedef struct {
    /* 0x0 */ s32 unk_00;
    /* 0x4 */ s32 unk_04;
} AreaPair; // size = 0x8

typedef struct {
    /* bit 0 */ u32 unk_0 : 1;
    /* bit 1 */ u32 unk_1 : 1;
    /* bit 2 */ u32 unk_2 : 1;
    /* bit 3 */ u32 unk_3 : 1;
    /* bit 4 */ u32 unk_4 : 2;
    /* bit 6 */ u32 unk_6 : 1;
    /* bit 7 */ u32 unk_7 : 25;
} AreaFlags;

/* One record per area of the current world, from the world data (.wdt) */
typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ AreaPair unk_08;
    /* 0x10 */ AreaFlags flags;
    /* 0x14 */ s32 unk_14;
    /* 0x18 */ s32 unk_18;
    /* 0x1C */ s32 unk_1C;
    /* 0x20 */ AreaPair unk_20;
    /* 0x28 */ AreaPair unk_28; // replaces unk_08 in Traverse Town later in the story
    /* 0x30 */ u8 unk_30[0x10];
} AreaInfo; // size = 0x40

/* Ways into an area of the current world */
typedef struct {
    /* 0x00 */ s32 area;
    /* 0x04 */ u8 unk_04[0x3C];
} AreaEntrance; // size = 0x40

typedef struct {
    /* 0x0 */ s32 infoOffset;
    /* 0x4 */ u32 infoSize;
    /* 0x8 */ s32 entranceOffset;
    /* 0xC */ u32 entranceSize;
} AreaTableHeader;

/* Header of the loaded world data: offsets from its start */
typedef struct {
    /* 0x00 */ u8 unk_00[0x10];
    /* 0x10 */ s32 areaTable;
    /* 0x14 */ s32 unk_14;
    /* 0x18 */ s32 unk_18;
    /* 0x1C */ s32 unk_1C;
    /* 0x20 */ s32 unk_20;
    /* 0x24 */ s32 unk_24[3];
    /* 0x30 */ s32 unk_30;
    /* 0x34 */ s32 unk_34;
    /* 0x38 */ s32 unk_38;
} WorldDataHeader;

extern s32 g_AreaWorld;
extern s32 g_AreaNumber;
typedef struct {
    /* bit 0 */ u32 unk_0 : 2;
    /* bit 2 */ u32 unk_2 : 1;
    /* bit 3 */ u32 unk_3 : 1;
    /* bit 4 */ u32 unk_4 : 1;
    /* bit 5 */ u32 unk_5 : 1;
    /* bit 6 */ u32 unk_6 : 26;
} SystemFlags;

extern SystemFlags g_SystemFlags;
extern s32 D_002BCC98;
extern AreaInfo* g_AreaInfos;

extern WorldDataHeader* g_WorldData;
extern void* D_002BCC90;
extern void* D_002BCC94;
extern void* D_002BCC9C;
extern void* D_002BCDE0;
extern u32 g_AreaInfoCount;
extern AreaEntrance* g_AreaEntrances;
extern u32 g_AreaEntranceCount;
extern s32 g_AreaEntranceIndex;
extern s32 D_002BCDD8;
extern s32 D_002BCDDC;
extern s32 D_002BC12C;
extern s32 D_002BC134;
extern s32 D_002BC734;
extern s32 D_002BCB30;
extern s32 D_002C12C0;
extern s32 D_002C1360;
extern s32 D_002C1380;
extern s32 D_002C1384;
extern AreaPair D_002A3F50;
extern AreaPair D_002C1378;
extern s32 D_002E27A4;

void Area_SetFileNames(void);
s32 func_00111660(s32 world, s32 area);
void func_001119D0(void);
void func_00103280(f32 a, f32 b, f32 c, f32 d);
void func_00110E38(f32 a, f32 b, f32 c, f32 d);
void func_001ED170(s32 arg0);
u8* func_001EAFF8(void);
void func_001124C0(void);

void func_00112498(void) {
    D_002BCC98 = g_AreaInfos[g_AreaNumber].unk_04;
}

void func_001124C0(void) {
    g_SystemFlags.unk_5 = 1;
}

void func_001124D8(void) {
    g_SystemFlags.unk_5 = 0;
}

/* Sets up the current area from the loaded world data: area table, entrance used,
 * per-area flags and parameters. */
void Area_Setup(void) {
    WorldDataHeader* wd = g_WorldData;
    AreaTableHeader* table;
    s32 temp;

    table = (AreaTableHeader*)((u8*)wd + wd->areaTable);
    D_002BCC90 = (u8*)wd + wd->unk_18;
    D_002BCC9C = (u8*)wd + wd->unk_20;
    D_002BCC94 = (u8*)wd + wd->unk_30;
    D_002BCDE0 = (u8*)wd + wd->unk_38;
    g_AreaInfos = (AreaInfo*)((u8*)table + table->infoOffset);
    g_AreaInfoCount = table->infoSize >> 6;
    g_AreaEntrances = (AreaEntrance*)((u8*)table + table->entranceOffset);
    g_AreaEntranceCount = table->entranceSize >> 6;

    if (D_002BCDD8 != 0) {
        g_AreaNumber = g_AreaEntrances[g_AreaEntranceIndex].area;
        D_002BC134 = func_00111660(g_AreaWorld, g_AreaNumber);
        Area_SetFileNames();
        if (D_002BCDDC == 0) {
            func_001119D0();
        }
    }

    D_002BC12C = g_AreaInfos[g_AreaNumber].unk_00;
    func_00103280(1.0f, 1.0f, 1.0f, 1.0f);
    func_00110E38(1.0f, 1.0f, 1.0f, 1.0f);
    D_002BC734 = g_AreaInfos[g_AreaNumber].flags.unk_1;
    if (D_002BC734 != 0) {
        func_00110E38(1.0f, 0.9f, 1.0f, 0.05f);
    }

    g_SystemFlags.unk_2 = !g_AreaInfos[g_AreaNumber].flags.unk_0;
    g_SystemFlags.unk_3 = !g_AreaInfos[g_AreaNumber].flags.unk_2;
    func_001124C0();

    D_002BCB30 = g_AreaInfos[g_AreaNumber].flags.unk_3;
    D_002C12C0 = g_AreaInfos[g_AreaNumber].flags.unk_6;
    temp = g_AreaInfos[g_AreaNumber].unk_14;
    D_002C1380 = temp;
    D_002C1360 = g_AreaInfos[g_AreaNumber].unk_18;
    D_002C1384 = g_AreaInfos[g_AreaNumber].unk_1C;
    D_002A3F50.unk_00 = g_AreaInfos[g_AreaNumber].unk_20.unk_00;
    D_002A3F50.unk_04 = g_AreaInfos[g_AreaNumber].unk_20.unk_04;
    func_001ED170(temp);

    D_002C1378.unk_00 = g_AreaInfos[g_AreaNumber].unk_08.unk_00;
    D_002C1378.unk_04 = g_AreaInfos[g_AreaNumber].unk_08.unk_04;
    if (g_AreaWorld == 3 && *func_001EAFF8() >= 0x2B) {
        D_002C1378.unk_00 = g_AreaInfos[g_AreaNumber].unk_28.unk_00;
        D_002C1378.unk_04 = g_AreaInfos[g_AreaNumber].unk_28.unk_04;
    }
    D_002E27A4 = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112820);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112840);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112928);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112968);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001129C8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112AD8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112B18);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112B48);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112B80);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112BB8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112C08);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112C20);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112CF8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112D28);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112D80);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112E08);

void func_00112E90(void) {
}

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112E98);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112FA0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112FC0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112FD0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112FE0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113008);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113028);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113048);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001130A0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001130E0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113138);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113270);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001133A0);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001134D8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113518);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113558);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113688);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001136A8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001136D8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113768);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001137C8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_001137F8);

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113A00);
