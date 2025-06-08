#include "io.h"
#include "hard_disk.h"

// Convierte letra de unidad a ID de BIOS (C: = 0x80, D: = 0x81, etc.)
static uint8_t drive_letter_to_bios(char letter) {
    return 0x80 + (letter - 'C');
}

// Convierte LBA a CHS para discos con 63 sectores/pista, 255 cabezas (estándar BIOS)
static void lba_to_chs(uint32_t lba, uint8_t& head, uint16_t& track, uint8_t& sector) {
    const uint8_t sectors_per_track = 63;
    const uint8_t heads_per_cylinder = 255;

    track  = lba / (sectors_per_track * heads_per_cylinder);
    uint32_t temp = lba % (sectors_per_track * heads_per_cylinder);
    head   = temp / sectors_per_track;
    sector = (temp % sectors_per_track) + 1;
}

bool read_sector_ata(uint32_t lba, void* buffer) {
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1); // Sector count
    outb(0x1F3, (uint8_t)(lba));
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20); // Read command

    // Esperar a que el disco esté listo
    while ((inb(0x1F7) & 0x80)); // Wait for BSY clear
    while (!(inb(0x1F7) & 0x08)); // Wait for DRQ set

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(0x1F0);
    }

    return true;
}
