#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAX_DISKS 4

struct DiskInfo {
    char letter;                     // 'A', 'B', etc.
    uint8_t drive_number;           // Para compatibilidad
    uint32_t sector_count;
    uint32_t sector_size;
    bool read_only;

    // Campos adicionales usados en ata_detect.cpp
    uint64_t capacity_bytes;
    bool read_write;
    char model[41];                 // Modelo del disco (IDE puede ser 40 chars)
    const char* channel;            // "Primary", "Secondary"
    const char* role;               // "Master", "Slave"
};

extern DiskInfo detected_disks[MAX_DISKS];
extern int detected_disk_count;

bool read_sector(const DiskInfo& disk, uint32_t lba, uint8_t* buffer);
void detect_disks();
