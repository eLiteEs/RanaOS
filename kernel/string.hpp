// kernel/string.hpp
#pragma once
#include <stddef.h>

namespace String {
    static inline void* memcpy(void* dest, const void* src, size_t n) {
        uint8_t* d = (uint8_t*)dest;
        const uint8_t* s = (const uint8_t*)src;
        while (n--) *d++ = *s++;
        return dest;
    }
    
    static inline int memcmp(const void* s1, const void* s2, size_t n) {
        const uint8_t* p1 = (const uint8_t*)s1;
        const uint8_t* p2 = (const uint8_t*)s2;
        for (; n--; p1++, p2++) {
            if (*p1 != *p2) {
                return *p1 - *p2;
            }
        }
        return 0;
    }

    static size_t strlen(const char* str) {
        size_t len = 0;
        while (str[len] != '\0') {
            len++;
        }
        return len;
    }
}