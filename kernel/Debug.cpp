#include "Debug.h"

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

namespace Debug {
    constexpr uint16_t COM1 = 0x3F8;

    void Init() {
        outb(COM1 + 1, 0x00);
        outb(COM1 + 3, 0x80);
        outb(COM1 + 0, 0x03);
        outb(COM1 + 1, 0x00);
        outb(COM1 + 3, 0x03);
        outb(COM1 + 2, 0xC7);
        outb(COM1 + 4, 0x0B);
    }

    static bool Ready() {
        return inb(COM1 + 5) & 0x20;
    }

    void PrintChar(char c) {
        while (!Ready());
        outb(COM1, c);
    }

    void Print(const char* s) {
        while (*s) PrintChar(*s++);
    }

    void PrintHex(uint32_t val) {
        const char* hex = "0123456789ABCDEF";
        for (int i = 28; i >= 0; i -= 4)
            PrintChar(hex[(val >> i) & 0xF]);
    }

    void PrintDec(uint32_t val) {
        char buf[10];
        int i = 0;
        do {
            buf[i++] = '0' + (val % 10);
            val /= 10;
        } while (val);
        while (i--) PrintChar(buf[i]);
    }
}
