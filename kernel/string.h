// kernel/include/string.h
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int strncmp(const char* s1, const char* s2, size_t n);
size_t strlen(const char* s);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
void* memmove(void* dst, const void* src, size_t n);

#ifdef __cplusplus
} // extern "C"
#endif