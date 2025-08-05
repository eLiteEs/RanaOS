#include "disk.h"

#define outb(port, value) asm volatile("outb %%al, %%dx" : : "a"(value), "d"(port))
#define outw(port, value) asm volatile("outw %%ax, %%dx" : : "a"(value), "d"(port))
#define inb(port) ({ uint8_t v; asm volatile("inb %%dx, %%al" : "=a"(v) : "d"(port)); v; })
#define inw(port) ({ uint16_t v; asm volatile("inw %%dx, %%ax" : "=a"(v) : "d"(port)); v; })

void ATA::wait_bsy() {
    while (inb(0x1F7) & 0x80);
}

void ATA::wait_drq() {
    while (!(inb(0x1F7) & 0x08));
}

void ATA::read_sectors(uint32_t lba, uint8_t count, void* buffer) {
    wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, count);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

    uint16_t* ptr = (uint16_t*)buffer;
    for (uint8_t i = 0; i < count; i++) {
        wait_bsy();
        wait_drq();
        for (int j = 0; j < 256; j++) {
            ptr[j] = inw(0x1F0);
        }
        ptr += 256;
    }
}

void ATA::write_sectors(uint32_t lba, uint8_t count, void* buffer) {
    wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, count);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);

    uint16_t* ptr = (uint16_t*)buffer;
    for (uint8_t i = 0; i < count; i++) {
        wait_bsy();
        wait_drq();
        for (int j = 0; j < 256; j++) {
            outw(0x1F0, ptr[j]);
        }
        ptr += 256;
    }
}

void ATA::identify_drive(uint16_t* data) {
    wait_bsy();
    outb(0x1F6, 0xA0);
    outb(0x1F7, 0xEC);
    
    while (1) {
        uint8_t status = inb(0x1F7);
        if (status & 0x01) return; // Error
        if (status & 0x08) break;  // DRQ
    }
    
    for (int i = 0; i < 256; i++) {
        data[i] = inw(0x1F0);
    }
}