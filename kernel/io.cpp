#include "Console.h"
#include "io.h"

extern "C" void putc(char c) {
    Console::putChar(c);
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1"
                  :
                  : "a"(val), "Nd"(port));
}
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
    return ret;
}