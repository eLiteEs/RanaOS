#pragma once
#include <stdint.h>

struct PartitionTableEntry {
    uint8_t status;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t sector_count;
} __attribute__((packed));

struct MBR {
    uint8_t bootstrap[446];
    PartitionTableEntry partitions[4];
    uint16_t signature;
} __attribute__((packed));

enum PartitionType {
    PARTITION_EMPTY = 0x00,
    PARTITION_FAT12 = 0x01,
    PARTITION_FAT16 = 0x04,
    PARTITION_FAT32 = 0x0B,
    PARTITION_FAT32_LBA = 0x0C,
    PARTITION_LINUX_SWAP = 0x82,
    PARTITION_LINUX_NATIVE = 0x83
};

const char* get_partition_type_name(uint8_t type);