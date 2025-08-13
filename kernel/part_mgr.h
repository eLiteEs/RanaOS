#pragma once
#include <stdint.h>
#include "mbr.h"

struct DiskInfo {
    uint32_t total_sectors;
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors_per_track;
    char model[41];
};

struct PartitionInfo {
    uint8_t status;
    uint8_t type;
    uint32_t start_lba;
    uint32_t sector_count;
    char type_name[32];
};

bool detect_disks(DiskInfo* disks, int max_count, int* count);
bool read_partitions(uint32_t disk_lba, PartitionInfo* partitions, int max_count, int* count);
bool create_partition(uint32_t disk_lba, int index, uint8_t type, uint32_t start_lba, uint32_t size_sectors);
bool delete_partition(uint32_t disk_lba, int index);
bool resize_partition(uint32_t disk_lba, int index, uint32_t new_size_sectors);
bool format_fat32(uint32_t partition_lba, uint32_t total_sectors);