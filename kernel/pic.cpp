#include "pic.h"
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void remap_pic() {
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA,    0x20);
    outb(PIC2_DATA,    0x28);
    outb(PIC1_DATA,    0x04);
    outb(PIC2_DATA,    0x02);
    outb(PIC1_DATA,    0x01);
    outb(PIC2_DATA,    0x01);
    // Al inicio enmascara todas líneas
    outb(PIC1_DATA,    0xFF);
    outb(PIC2_DATA,    0xFF);
}

void enable_irq(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    uint8_t mask = inb(port) & ~(1 << irq);
    outb(port, mask);
}

