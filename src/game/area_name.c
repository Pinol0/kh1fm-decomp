/*
 * File names of world and area data.
 *
 * Every world has a two-letter prefix (tw = Traverse Town, ...). From it and an area
 * number this file builds "tw.wdt" (world data), "tw03.ard" (area archive), "tw03"
 * (area base name) and "tw00_01" (base name of the .bin/.img pairs).
 */
#include "common.h"
#include "libc.h"

extern char* g_WorldPrefixes[];
extern s32 g_AreaWorld;
extern s32 g_AreaNumber;
extern char g_WorldDataName[0x40];
extern char g_AreaBinImgName[0x40];
extern char g_AreaBaseName[0x40];
extern char g_AreaArchiveName[0x40];

/* Still in the asm rodata/data blocks */
extern char D_0048CFF8[]; /* "%2d" */
extern char D_0048D000[]; /* "00_" */
extern char D_002BCC80[]; /* ".wdt" */
extern char D_002BCC88[]; /* ".ard" */

char* Area_GetWorldPrefix(s32 world, char* dest) {
    return strcpy(dest, g_WorldPrefixes[world]);
}

void Area_MakeName(s32 world, s32 area, s32 unused, char* dest) {
    char number[0x10];
    char prefix[0x40];

    Area_GetWorldPrefix(world, prefix);
    sprintf(number, D_0048CFF8, area + 1);
    if (number[0] == ' ') {
        number[0] = '0';
    }
    strcpy(dest, prefix);
    strcat(dest, number);
}

void Area_SetFileNames(void) {
    char number[0x10];
    char prefix[0x40];

    Area_GetWorldPrefix(g_AreaWorld, prefix);
    sprintf(number, D_0048CFF8, g_AreaNumber + 1);
    if (number[0] == ' ') {
        number[0] = '0';
    }
    strcpy(g_WorldDataName, prefix);
    strcat(g_WorldDataName, D_002BCC80);
    strcpy(g_AreaArchiveName, prefix);
    strcat(g_AreaArchiveName, number);
    strcat(g_AreaArchiveName, D_002BCC88);
    strcpy(g_AreaBaseName, prefix);
    strcat(g_AreaBaseName, number);
}

void Area_SetBinImgName(s32 index) {
    char number[0x10];
    char prefix[0x40];

    Area_GetWorldPrefix(g_AreaWorld, prefix);
    sprintf(number, D_0048CFF8, index + 1);
    if (number[0] == ' ') {
        number[0] = '0';
    }
    strcpy(g_AreaBinImgName, prefix);
    strcat(g_AreaBinImgName, D_0048D000);
    strcat(g_AreaBinImgName, number);
}
