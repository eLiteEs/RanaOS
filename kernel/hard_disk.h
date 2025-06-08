#pragma once

#include <stdint.h>

bool read_sector(char drive_letter, uint32_t lba, void* buffer);

