#include "disk.hpp"
#include "io.h"  // Para inb/outb
#include "Console.h"

// Cosas del PIT
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

void wait_ms(uint32_t ms) {
    if (ms == 0) return;

    // Each iteration is limited to 54.925 ms (maximum divisor = 65535)
    while (ms > 0) {
        uint32_t chunk = (ms > 54) ? 54 : ms;
        ms -= chunk;

        uint16_t divisor = (uint16_t)(1193182 / 1000 * chunk); // = 1193 * chunk

        // Set PIT channel 0 to mode 0 (one-shot), binary counting
        outb(PIT_COMMAND, 0b00110100); // channel 0, access lobyte/hibyte, mode 0

        // Load divisor
        outb(PIT_CHANNEL0, divisor & 0xFF);        // low byte
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF); // high byte

        // Wait until the countdown is done (OUT == 1)
        while (true) {
            outb(PIT_COMMAND, 0xE2); // latch status of channel 0
            uint8_t status = inb(PIT_CHANNEL0);
            if (status & (1 << 7)) break; // OUT = 1, finished
        }
    }
}

void Disk::WaitBSY() {
    while (inb(0x1F7) & 0x80); // Esperar a que el disco no esté ocupado
}

void Disk::WaitDRQ() {
    while (!(inb(0x1F7) & 0x08)); // Esperar a que esté listo para transferir
}

static inline void io_wait() {
    asm volatile ("outb %%al, $0x80" : : "a"(0)); // Delay en puerto no usado
}

bool ReadBootSector(uint8_t* buffer) {
    // Intenta leer de ambos discos (maestro y esclavo)
    for (int i = 0; i < 2; i++) {
        outb(0x1F6, 0xE0 | (i << 4)); // Seleccionar disco
        io_wait();
        
        if (!Disk::ReadSector(0, buffer)) continue;
        
        // Verificar firma MBR
        if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
            Console::println("MBR encontrado en disco ");
            Console::write(i == 0 ? "maestro" : "esclavo");
            return true;
        }
    }
    return false;
}

bool Disk::ReadSector(uint32_t lba, uint8_t* buffer) {
    // 1. Esperar disco listo
    uint32_t timeout = 1000000;
    while (((inb(0x1F7) & 0x80) != 0) && (--timeout != 0));
    if (!timeout) {
        Console::println("Timeout esperando BSY=0");
        return false;
    }

    // 2. Configurar LBA
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));  // LBA28, disco maestro
    io_wait();
    outb(0x1F2, 1);  // Leer 1 sector
    io_wait();
    outb(0x1F3, lba & 0xFF);  // LBA bits 0-7
    io_wait();
    outb(0x1F4, (lba >> 8) & 0xFF);  // LBA bits 8-15
    io_wait();
    outb(0x1F5, (lba >> 16) & 0xFF);  // LBA bits 16-23
    io_wait();

    // 3. Enviar comando READ
    outb(0x1F7, 0x20);
    io_wait();
    wait_ms(1); // Pequeño delay

    // 4. Esperar datos listos
    timeout = 1000000;
    while (!(inb(0x1F7) & 0x08) && --timeout) { // Esperar DRQ=1
        if (inb(0x1F7) & 0x01) { // Error
            Console::println("Error en disco durante lectura");
            return false;
        }
    }
    if (!timeout) {
        Console::println("Timeout esperando DRQ");
        return false;
    }

    // 5. Leer 256 palabras (512 bytes)
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i*2] = data & 0xFF;
        buffer[i*2+1] = (data >> 8) & 0xFF;
    }

    return true;
}

bool Disk::WriteSector(uint32_t lba, uint8_t* buffer) {
    WaitBSY();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); // Seleccionar unidad (LBA)
    outb(0x1F2, 1);                           // Escribir 1 sector
    outb(0x1F3, lba & 0xFF);                  // LBA low
    outb(0x1F4, (lba >> 8) & 0xFF);           // LBA mid
    outb(0x1F5, (lba >> 16) & 0xFF);          // LBA high
    outb(0x1F7, 0x30);                        // Comando WRITE

    WaitDRQ();
    for (int i = 0; i < 256; i++) {
        uint16_t data = (buffer[i * 2 + 1] << 8) | buffer[i * 2];
        outw(0x1F0, data);
    }

    return true;
}

bool Disk::CheckReady() {
    // Esperar máximo 30 intentos
    for (int i = 0; i < 30; i++) {
        uint8_t status = inb(0x1F7); // Puerto de estado ATA
        if (!(status & 0x80)) return true; // BSY=0
        if (status & 0x01) return false; // ERROR=1
    }
    return false;
}

bool Disk::CheckExists() {
    outb(0x1F6, 0xE0); // Seleccionar disco maestro
    io_wait();
    outb(0x1F7, 0xEC); // Comando IDENTIFY
    io_wait();
    
    uint32_t timeout = 1000000;
    while ((inb(0x1F7) & 0x80) && --timeout) io_wait(); // Esperar BSY=0
    
    if (!timeout || inb(0x1F7) == 0) {
        Console::println("Disco no responde o no existe");
        return false;
    }
    return true;
}