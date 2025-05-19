// floppy.h
#pragma once
#include <stdint.h>

#define FLOPPY_SECTOR_SIZE 512

// Lee un solo sector LBA
bool floppy_read_sector(uint32_t lba, void* buffer);

// Lee 'count' sectores consecutivos a partir de LBA 0
bool floppy_read_image(uint32_t count, void* buffer);

