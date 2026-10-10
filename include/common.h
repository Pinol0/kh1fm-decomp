#ifndef COMMON_H
#define COMMON_H

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long s64;
typedef unsigned long u64;
typedef float f32;
#ifndef M2CTX
typedef unsigned int u128 __attribute__((mode(TI))); /* 128-bit GPR (lq / sq) */
#else
typedef struct { u64 lo, hi; } u128;
#endif

#define NULL ((void*)0)

#include "include_asm.h"

#endif
