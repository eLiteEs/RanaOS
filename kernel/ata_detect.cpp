#include <stdint.h>
#include "io.h"
#include "console.h"
#include "disk.h"

uint16_t ata_read_word(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Read model string from words 27-46
void read_model(uint16_t* data, char* buffer, int buffer_size) {
    int index = 0;
    for (int i = 27; i <= 46 && index + 1 < buffer_size; ++i) {
        uint16_t word = data[i];
        buffer[index++] = (word >> 8) & 0xFF;
        buffer[index++] = word & 0xFF;
    }
    buffer[index] = '\0';

    // Trim trailing spaces
    for (int i = index - 1; i >= 0; --i) {
        if (buffer[i] == ' ') buffer[i] = '\0';
        else break;
    }
}

// Sends IDENTIFY command and extracts disk info
bool identify_disk(uint16_t io_base, bool is_slave, DiskInfo& info) {
    outb(io_base + 6, is_slave ? 0xB0 : 0xA0);
    for (int i = 0; i < 100000; ++i) asm volatile("nop");

    outb(io_base + 7, 0xEC); // IDENTIFY command

    uint8_t status = inb(io_base + 7);
    if (status == 0 || status == 0xFF) return false;

    while ((status & 0x80) && !(status & 0x08))
        status = inb(io_base + 7);

    if (!(status & 0x08)) return false;

    uint16_t data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = ata_read_word(io_base);

    uint32_t total_sectors = data[60] | ((uint32_t)data[61] << 16);
    info.capacity_bytes = (uint64_t)total_sectors * 512;
    info.read_write = true;

    read_model(data, info.model, sizeof(info.model));
    return true;
}

// Main disk detection function
void detect_disks() {
    const uint16_t channels[] = {0x1F0, 0x170};
    const char* names[] = {"Primary", "Secondary"};
    char letters[] = {'C', 'D', 'E', 'F'};
    int letter_index = 0;

    for (int i = 0; i < 2; ++i) {
        for (int slave = 0; slave < 2; ++slave) {
            DiskInfo disk;
            disk.channel = names[i];
            disk.role = slave ? "Slave" : "Master";
            disk.letter = letters[letter_index];

            if (identify_disk(channels[i], slave, disk)) {
                Console::println(
                    "Disk ", disk.letter, ": ",
                    disk.model,
                    " - ", disk.channel, " ", disk.role,
                    " - Capacity: ", disk.capacity_bytes, " bytes (",
                    disk.capacity_bytes / 1048576.0, " MB)",
                    " - ", disk.read_write ? "Read/Write" : "Read Only"
                );
                letter_index++;
            }
        }
    }

    if (letter_index == 0) {
        Console::println("No disks detected, comrade.");
    }
}

