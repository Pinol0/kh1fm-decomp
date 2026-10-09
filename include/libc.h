#ifndef LIBC_H
#define LIBC_H

/* C library functions linked from the SDK's libc (see sdk/libc in config/kh1fm.yaml). */

char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int sprintf(char* str, const char* format, ...);
void* memset(void* dest, int value, unsigned int size);

#endif
