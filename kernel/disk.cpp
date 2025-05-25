#include "disk.h"
#include "floppy.h"

#define FLOPPY_TOTAL_SECTORS 2880

DiskInfo detected_disks[MAX_DISKS];
int detected_disk_count = 0;

bool read_sector(const DiskInfo& disk, uint32_t lba, uint8_t* buffer) {
    if (lba >= disk.sector_count) return false;
    return floppy_read_sector(lba, buffer);
}

