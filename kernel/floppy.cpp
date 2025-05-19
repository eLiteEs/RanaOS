// floppy.cpp
#include "floppy.h"
#include "io.h"       // declara extern inb()/outb()
#include <stdint.h>

// Puertos del controlador floppy
#define FDC_BASE        0x3F0
#define FDC_DOR         (FDC_BASE + 2)
#define FDC_MSR         (FDC_BASE + 4)
#define FDC_DATA        (FDC_BASE + 5)

// Tamaño de sector en bytes
#define FLOPPY_SECTOR_SIZE 512

// Espera breve tras cada E/S
static inline void io_wait() {
    for (volatile int i = 0; i < 1000; ++i);
}

// Espera hasta que el FDC esté listo para recibir comandos
static void wait_ready() {
    for (int i = 0; i < 500000; ++i) {
        if ((inb(FDC_MSR) & 0xC0) == 0x80) return;
        io_wait();
    }
    // Aquí podrías agregar manejo de timeout/error
}

// Polling simple en lugar de IRQ real
static void wait_irq() {
    for (volatile int i = 0; i < 1000000; ++i);
}

// Resetea y enciende la controladora y el motor
static void reset_fdc() {
    outb(FDC_DOR, 0x00);
    for (volatile int i = 0; i < 10000; ++i);
    outb(FDC_DOR, 0x0C);
    for (volatile int i = 0; i < 10000; ++i);
}

// Convierte LBA a CHS para floppy 1.44 MB (18 sectores/pista, 2 caras)
static void lba_to_chs(uint32_t lba, uint8_t &head, uint8_t &track, uint8_t &sector) {
    head   = (lba / 18) % 2;
    track  = (lba / 36);
    sector = (lba % 18) + 1;
}

// Lee un sector (512 B) desde el floppy en modo PIO
bool floppy_read_sector(uint32_t lba, void* buffer) {
    uint8_t head, track, sector;
    lba_to_chs(lba, head, track, sector);

    reset_fdc();

    wait_ready(); outb(FDC_DATA, 0xE6);      // Comando READ SECTOR (MFM, multi, MT, SK)
    wait_ready(); outb(FDC_DATA, track);    // cilindro
    wait_ready(); outb(FDC_DATA, head);     // cabeza
    wait_ready(); outb(FDC_DATA, sector);   // sector (1–18)
    wait_ready(); outb(FDC_DATA, 2);        // tamaño = 2 => 512 bytes
    wait_ready(); outb(FDC_DATA, 18);       // sectores por pista
    wait_ready(); outb(FDC_DATA, 0x1B);     // GAP3 length
    wait_ready(); outb(FDC_DATA, 0xFF);     // cantidad de bytes (unused)

    wait_irq();  // espera a que termine la lectura

    // Transferencia PIO: leer 512 B del FIFO
    uint8_t* dst = (uint8_t*)buffer;
    for (int i = 0; i < FLOPPY_SECTOR_SIZE; ++i) {
        dst[i] = inb(FDC_DATA);
    }

    // Limpiar los 7 bytes de status/resultados
    for (int i = 0; i < 7; ++i) {
        inb(FDC_DATA);
    }

    return true;
}

// Lee 'count' sectores consecutivos (desde LBA 0) al buffer
bool floppy_read_image(uint32_t count, void* buffer) {
    uint8_t* ptr = (uint8_t*)buffer;
    for (uint32_t i = 0; i < count; ++i) {
        if (!floppy_read_sector(i, ptr + i * FLOPPY_SECTOR_SIZE))
            return false;
    }
    return true;
}

