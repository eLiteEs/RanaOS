#include "Console.h"
#include "io.h"
#include <stdint.h>

extern "C" {

// Lee un byte desde un puerto
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
    return ret;
}

// Escribe un byte a un puerto
void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1"
                  :
                  : "a"(value), "Nd"(port));
}

// Lee una palabra (2 bytes) desde un puerto
uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
    return ret;
}

// Escribe una palabra (2 bytes) a un puerto
void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1"
                  :
                  : "a"(val), "Nd"(port));
}

}
