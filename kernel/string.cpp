// kernel/string.cpp
#include "string.h"
#include <stdint.h> // Para uint8_t

// Implementaciones
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) s1++, s2++;
    return n == (size_t)-1 ? 0 : *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

char* strchr(const char* s, int c) {
    while (*s && *s != c) s++;
    return *s == c ? (char*)s : nullptr;
}

char* strrchr(const char* s, int c) {
    const char* last = nullptr;
    while (*s) {
        if (*s == c) last = s;
        s++;
    }
    return (char*)last;
}

void* memmove(void* dst, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d == s) return dst;
    
    // Si las regiones se solapan y dst está después de src, copiamos de atrás hacia adelante
    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    } else {
        // Copia normal de adelante hacia atrás
        while (n--) {
            *d++ = *s++;
        }
    }
    
    return dst;
}

void strncpy(char* dest, const char* src, int maxLen) {
    int i = 0;
    for (; i < maxLen - 1 && src[i]; ++i)
        dest[i] = src[i];
    dest[i] = 0;
}

/*char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}*/