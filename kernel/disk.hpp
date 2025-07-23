#pragma once
#include <stdint.h>

void wait_ms(uint32_t ms);

class Disk {
public:
    static bool ReadSector(uint32_t lba, uint8_t* buffer);
    static bool WriteSector(uint32_t lba, uint8_t* buffer);
    static bool CheckReady();
    static bool CheckExists();
    static bool ReadBootSector(uint8_t* buffer);

private:
    static void WaitBSY();
    static void WaitDRQ();
};