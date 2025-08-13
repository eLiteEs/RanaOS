#pragma once
#include <stdint.h>

class ATA {
public:
    static void wait_bsy();
    static void wait_drq();
    static void read_sectors(uint32_t lba, uint8_t count, void* buffer);
    static void write_sectors(uint32_t lba, uint8_t count, void* buffer);
    static void identify_drive(uint16_t* data);
};