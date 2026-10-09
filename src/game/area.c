/*
 * Area management: per-area info table, loading state and related flags.
 */
#include "common.h"
#include "libc.h"

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

typedef struct {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
    /* 0xC */ f32 rotY;
} AreaActorPlace; // size = 0x10

/* Ways into an area of the current world */
typedef struct {
    /* 0x00 */ s32 area;
    /* 0x04 */ u8 unk_04[0xC];
    /* 0x10 */ AreaActorPlace spawn[3]; // party members: 0 = Sora
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
/* Area change sequence: each step queues the next one through func_0011EF10 */
s32 func_0011EF10(s32 arg0, void* callback);
s32 func_001017E8(void);
void func_0011CB08(char* arg0);
s32 func_001559F0(s32 arg0, s32 arg1);
void func_00120748(char* arg0, s32 arg1, void* callback, s32 arg3);
void func_0010E750(void);
void func_0010B030(void* arg0);
void* func_001137C8(void* arg0, s32 arg1);
void func_001C1F60(void);
void func_00122330(void);
void func_00100AE0(void);
void func_00111600(s32 world, s32 area, s32 arg2, s32 entrance);
s32 func_0011C898(s32 arg0);
void func_00114820(void);
void func_00109558(s32 arg0);
s32 func_00112840(s32 arg0, s32 arg1, s32 arg2);
s32 func_001129C8(void);
void func_00112968(void);
s32 func_00112AD8(void);
void func_00112928(void);
void func_00112B18(void);
void func_00112FD0(s32 arg0);

void func_00157668(void);
void func_0011C7C0(s32 arg0, s32 arg1, s32 arg2);
void func_0011D4E0(s32 arg0);
void func_00101728(s32 arg0);
void func_001C2EC8(s32 arg0, s32 arg1);
void func_00110360(void);
void func_0013C078(void);
void func_00101680(void);
s32 func_00113558(void);
void func_00113688(void);
s32 func_001136D8(void);
void func_00113518(void);
void func_00112498(void);

/* Part of an actor (party member) used here */
typedef struct {
    /* 0x00 */ u8 unk_00[0x10];
    /* 0x10 */ f32 posX;
    /* 0x14 */ f32 posY;
    /* 0x18 */ f32 posZ;
    /* 0x1C */ u8 unk_1C[0x20];
    /* 0x3C */ f32 rotY;
} AreaActor;


extern AreaActor* D_002E26A0[3];
void func_001EC7D0(s32 arg0);
void func_001ED128(s32 arg0, s32 arg1);
void func_00101628(void);
void func_0014A090(void);
void func_00146928(s32 arg0);
s32 func_0011CDF0(s32 arg0, s32 arg1, void* callback);
void func_00112820(void);
void func_00176170(void);
void func_001761B0(void);
extern char g_WorldDataName[0x40];
extern u8 g_WorldDataBuffer[]; /* fixed buffer at 0x9A0000 */
extern s32 D_002BC138; /* world of the world data in memory */
extern s32 D_002BCDB4;
extern s32 D_002BCDB8;
extern s32 D_002BCDC4;

typedef struct {
    /* 0x0 */ s32 world;
    /* 0x4 */ s32 area;
} AreaLocation;

typedef struct {
    /* 0x0 */ s32 world;
    /* 0x4 */ s32 area;
    /* 0x8 */ s32 entrance;
} AreaLocationEntrance;

extern AreaLocation D_002BC148;
extern AreaLocationEntrance D_002BCDC8;
typedef struct {
    /* bit 0  */ u32 unk_0 : 13;
    /* bit 13 */ u32 unk_13 : 1;
    /* bit 14 */ u32 unk_14 : 5;
    /* bit 19 */ u32 unk_19 : 1;
    /* bit 20 */ u32 unk_20 : 12;
} GameFlags;

extern GameFlags D_002C5958;
extern s32 D_002BC13C; /* id of the .bin/.img data currently loaded */
extern s32 D_002BCDBC;
extern void* D_002A3F3C;
extern s32 D_002BCDE8;
extern s32 D_002C1388;
extern char g_AreaBinImgName[0x40];
extern char D_0048D008[]; /* ".img" */
void Area_SetBinImgName(s32 index);
void func_0012CAC8(s32 arg0, s32 arg1);
void func_00112D28(void);
void func_00113048(void);
s32 func_00112CF8(void);
void func_00112C08(s32 arg0);
void func_0013C6D8(void);
s32 func_001148C8(void);
s32 func_0011E9A8(void* callback, s32 arg1);
s32 func_001136A8(void);
s32 func_00122198(void* arg0, void* callback);
extern s32 D_002BBE1C;
void func_00104FD0(s32 member, f32* dest);
void func_00111590(s32 member, f32* dest);
void func_001130A0(s32 index, f32* dest);
s32 func_001130E0(void);
extern AreaActorPlace D_004D92F0[3];
extern s32 D_002BCDE4;
extern s32 D_002BCDEC;
extern void* D_002BCDF0;
extern char g_AreaArchiveName[0x40];
extern s32 D_004D934C;
extern s32 D_004D9350;
extern s32 D_004D9354;
extern s32 D_004D9358;
extern s32 D_004D935C;
extern s32 D_004D9360;
extern void* D_002A3F38;
extern s32 D_002A3F40;
extern s32 D_002B6474;
extern s32 D_002BCDC0;
extern s32 D_004D92E0;

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

void func_00112820(void) {
    func_0011EF10(50000, func_001017E8);
}

s32 func_00112840(s32 arg0, s32 arg1, s32 arg2) {
    func_001EC7D0(2);
    func_001ED128(3, -9);
    func_00101628();
    func_001ED128(0, -1);
    func_001ED128(3, -1);
    func_001ED128(0, 0);
    func_001ED128(0, 1);
    func_001ED128(1, 0);
    func_001ED128(1, 1);
    func_0014A090();
    func_00146928(-3);
    func_00146928(-4);
    func_00146928(-5);
    func_00146928(-6);
    func_00146928(-7);
    func_001ED128(0, -8);
    func_001ED128(3, -8);
    func_001ED128(3, -10);
    return func_0011CDF0(-1, arg2, func_00112820);
}

void func_00112928(void) {
    char buf[0x40];

    func_0011CB08(buf);
    func_00120748(buf, func_001559F0(0x34, 4), func_00112840, 0);
}

void func_00112968(void) {
    Area_Setup();
    func_0010E750();
    func_0010B030(D_002BCDE0);
    D_002A3F38 = D_002BCDE0;
    D_002A3F38 = func_001137C8(D_002BCDE0, 0x40);
    func_00112928();
}

/* Area change step: (re)loads the world data unless the world is unchanged */
s32 func_001129C8(void) {
    if (D_002BCDBC == 1 && D_002BCDC0 == 0) {
        return 0;
    }
    Area_SetFileNames();
    D_002BCDB4 = 0;
    D_002BCDB8 = 0;
    D_002BCDC4 = 1;
    g_WorldData = (WorldDataHeader*)g_WorldDataBuffer;
    func_00176170();
    func_001761B0();
    if (D_002BC138 == g_AreaWorld) {
        D_002BCDB4 = 1;
    }
    if (D_002BCDB4 == 1) {
        Area_Setup();
        if (D_002BCDD8 != 0) {
            func_00112928();
        } else {
            func_00112820();
        }
    } else {
        func_00120748(g_WorldDataName, (s32)g_WorldData, func_00112968, 0);
    }
    return 4;
}

s32 func_00112AD8(void) {
    func_001C1F60();
    D_002B6474 = 0;
    func_00122330();
    func_0011EF10(0x2E62F, func_001129C8);
    return 4;
}

void func_00112B18(void) {
    func_00100AE0();
    func_0011EF10(0x2E630, func_00112AD8);
}

void func_00112B48(s32 world, s32 area, s32 arg2, s32 entrance) {
    g_AreaEntranceIndex = entrance;
    g_AreaWorld = world;
    g_AreaNumber = area;
    D_002BC134 = arg2;
    func_00111600(world, area, arg2, entrance);
    func_00112B18();
}

void func_00112B80(s32 world, s32 area, s32 arg2, s32 entrance) {
    g_AreaEntranceIndex = entrance;
    g_AreaWorld = world;
    g_AreaNumber = area;
    D_002BC134 = arg2;
    func_00112B18();
}

void func_00112BB8(s32 entrance) {
    g_AreaEntranceIndex = entrance;
    g_AreaNumber = g_AreaEntrances[entrance].area;
    D_002BC134 = func_00111660(g_AreaWorld, g_AreaNumber);
    func_00112B18();
}

void func_00112C08(s32 arg0) {
    D_002A3F40 = arg0;
    D_002BCDC0 = 1;
}

/* Starts loading the .bin/.img data of the area behind an entrance, unless already loaded */
void func_00112C20(s32 entrance) {
    char name[0x40];
    s32 id;

    if (D_002C5958.unk_13 != 1) {
        id = g_AreaInfos[g_AreaEntrances[entrance].area].unk_00;
        if (D_002BC13C != id) {
            D_002BCDBC = 1;
            Area_SetBinImgName(id);
            strcpy(name, g_AreaBinImgName);
            strcat(name, D_0048D008);
            D_002A3F3C = (void*)func_001559F0(0x34, 8);
            D_002A3F3C = func_001137C8(D_002A3F3C, 0x80);
            func_00120748(name, (s32)D_002A3F3C, func_00112C08, 0);
        }
    }
}

s32 func_00112CF8(void) {
    func_0011C898(0);
    D_002B6474 = 0;
    func_00122330();
    return 4;
}

void func_00112D28(void) {
    func_00157668();
    D_002A3F50.unk_04 = 0;
    D_002A3F50.unk_00 = 0;
    func_0011C7C0(0, 0, 360);
    func_0011C7C0(1, 0, 360);
    func_0011D4E0(360);
    func_00101728(360);
}

void func_00112D80(void) {
    func_0012CAC8(2, 0);
    func_0012CAC8(6, 0);
    D_002C5958.unk_19 = 1;
    func_00112D28();
    func_001C1F60();
    func_0011EF10(0x2E630, func_00112CF8);
    D_002BC148.world = g_AreaWorld;
    D_002BC148.area = g_AreaNumber;
    func_00109558(4);
}

void func_00112E08(void) {
    D_002C5958.unk_19 = 1;
    func_00112D28();
    func_001C1F60();
    func_00113048();
    func_0011EF10(0x2E630, func_00112CF8);
    D_002BCDC8.world = g_AreaWorld;
    D_002BCDC8.area = g_AreaNumber;
    D_002BCDC8.entrance = g_AreaEntranceIndex;
    func_00109558(6);
}

void func_00112E90(void) {
}

INCLUDE_ASM("asm/nonmatchings/game/area", func_00112E98);

void func_00112FA0(void) {
    func_00114820();
    func_00109558(9);
}

s32 func_00112FC0(void) {
    return D_004D92E0;
}

void func_00112FD0(s32 arg0) {
    D_004D92E0 = arg0;
}

void func_00112FE0(void) {
    func_00114820();
    func_00112FD0(1);
    func_00109558(8);
}

void func_00113008(void) {
    func_001559F0(0x3D, 3);
}

void func_00113028(void) {
    func_001559F0(0, 0x40);
}

void func_00113048(void) {
    AreaActor* a;
    s32 i;

    for (i = 0; i < 3; i++) {
        a = D_002E26A0[i];
        if (a != NULL) {
            D_004D92F0[i].x = a->posX;
            D_004D92F0[i].y = a->posY;
            D_004D92F0[i].z = a->posZ;
            D_004D92F0[i].rotY = a->rotY;
        }
    }
}

#ifdef NON_MATCHING
// Copies the saved place of party member `index` to dest. Equivalent; the address
// register copies differ.
void func_001130A0(s32 index, f32* dest) {
    dest[0] = D_004D92F0[index].x;
    dest[1] = D_004D92F0[index].y;
    dest[2] = D_004D92F0[index].z;
    dest[3] = D_004D92F0[index].rotY;
}
#else
INCLUDE_ASM("asm/nonmatchings/game/area", func_001130A0);
#endif

s32 func_001130E0(void) {
    s32 i;

    for (i = 0; i < (s32)g_AreaEntranceCount; i++) {
        if (g_AreaNumber == g_AreaEntrances[i].area) {
            return i;
        }
    }
    return 1000;
}

#ifdef NON_MATCHING
// Equivalent; one branch is emitted as bnel instead of bne (delay slot filling).
void func_00113138(f32* dest) {
    s32 i = g_AreaEntranceIndex;

    if (i == -1) {
        i = g_AreaEntranceIndex = func_001130E0();
    }
    if (i < (s32)g_AreaEntranceCount) {
        dest[0] = g_AreaEntrances[i].spawn[0].x;
        dest[1] = g_AreaEntrances[g_AreaEntranceIndex].spawn[0].y;
        dest[2] = g_AreaEntrances[g_AreaEntranceIndex].spawn[0].z;
        dest[3] = g_AreaEntrances[g_AreaEntranceIndex].spawn[0].rotY;
        if (D_002BCDD8 != 0) {
            if (D_002BCDDC == 1) {
                func_001130A0(0, dest);
            }
        }
        if (D_002BBE1C == 1) {
            func_00111590(0, dest);
        }
    } else {
        func_00104FD0(0, dest);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/game/area", func_00113138);
#endif

#ifdef NON_MATCHING
// Equivalent; one branch is emitted as bnel instead of bne (delay slot filling).
void func_00113270(f32* dest) {
    s32 i = g_AreaEntranceIndex;

    if (i == -1) {
        i = g_AreaEntranceIndex = func_001130E0();
    }
    if (i < (s32)g_AreaEntranceCount) {
        dest[0] = g_AreaEntrances[i].spawn[1].x;
        dest[1] = g_AreaEntrances[g_AreaEntranceIndex].spawn[1].y;
        dest[2] = g_AreaEntrances[g_AreaEntranceIndex].spawn[1].z;
        dest[3] = g_AreaEntrances[g_AreaEntranceIndex].spawn[1].rotY;
        if (D_002BCDD8 != 0) {
            if (D_002BCDDC == 1) {
                func_001130A0(1, dest);
            }
        }
        if (D_002BBE1C == 1) {
            func_00111590(1, dest);
        }
    } else {
        func_00104FD0(0, dest);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/game/area", func_00113270);
#endif

#ifdef NON_MATCHING
// Equivalent; one branch is emitted as bnel instead of bne (delay slot filling).
void func_001133A0(f32* dest) {
    s32 i = g_AreaEntranceIndex;

    if (i == -1) {
        i = g_AreaEntranceIndex = func_001130E0();
    }
    if (i < (s32)g_AreaEntranceCount) {
        dest[0] = g_AreaEntrances[i].spawn[2].x;
        dest[1] = g_AreaEntrances[g_AreaEntranceIndex].spawn[2].y;
        dest[2] = g_AreaEntrances[g_AreaEntranceIndex].spawn[2].z;
        dest[3] = g_AreaEntrances[g_AreaEntranceIndex].spawn[2].rotY;
        if (D_002BCDD8 != 0) {
            if (D_002BCDDC == 1) {
                func_001130A0(2, dest);
            }
        }
        if (D_002BBE1C == 1) {
            func_00111590(2, dest);
        }
    } else {
        func_00104FD0(0, dest);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/game/area", func_001133A0);
#endif

void func_001134D8(s32 arg0) {
    D_002BCDE4 = arg0;
    if (arg0 != 0) {
        D_002BCDEC = 0;
        func_001C2EC8(arg0, 0);
    } else {
        func_00110360();
    }
}

void func_00113518(void) {
    if (D_002BCDD8 != 0 && D_002BCDDC == 0) {
        func_0013C078();
    }
}

INCLUDE_ASM("asm/nonmatchings/game/area", func_00113558);

void func_00113688(void) {
    func_0011EF10(49900, func_00113558);
}

s32 func_001136A8(void) {
    func_00113518();
    return func_00122198(D_002BCDF0, func_00113688);
}

s32 func_001136D8(void) {
    if (D_002B6474 == 1) {
        func_0013C6D8();
    }
    func_001148C8();
    if (D_002BCDE8 == 0) {
        D_002C1388 = 1;
        return func_0011E9A8(func_001136A8, func_001559F0(0x34, 4));
    }
    return func_001136A8();
}

s32 func_00113768(void) {
    func_00112498();
    func_00101680();
    D_002BCDF0 = func_001137C8(D_002BCDF0, 0x80);
    func_00120748(g_AreaArchiveName, (s32)D_002BCDF0, func_001136D8, 0);
    do { return 4; } while (0); // TODO fake match
}

#ifdef NON_MATCHING
// Rounds addr up to a multiple of align. Equivalent; mflo is scheduled before the branch
// instead of in its delay slot.
void* func_001137C8(void* addr, s32 align) {
    return ((s32)addr % align) ? (void*)((s32)addr / align * align + align) : addr;
}
#else
INCLUDE_ASM("asm/nonmatchings/game/area", func_001137C8);
#endif

INCLUDE_ASM("asm/nonmatchings/game/area", func_001137F8);

void func_00113A00(void) {
    D_004D934C = 0xCD;
    D_004D9350 = 0x87;
    D_004D9354 = 0x32;
    D_004D9358 = 0x80;
    D_004D935C = 0x4C;
    D_004D9360 = 0x80;
}
