#pragma once
#include <stdint.h>
#include "disk.hpp"
#include "string.hpp"

class FileSystem {
public:
    static bool Initialize();
    static bool ReadFile(const char* name, uint8_t* buffer);
    static bool WriteFile(const char* name, const uint8_t* data, uint32_t size);
    static bool DeleteFile(const char* name);
    static void ListFiles();
    static void CreateBasicMBR(uint8_t* sector);

private:
    #pragma pack(push, 1)
    struct BootSector {
        uint8_t  jump[3];
        char     oem[8];
        uint16_t bytes_per_sector;
        uint8_t  sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t  fat_count;
        uint16_t root_entries;
        uint16_t total_sectors_16;
        uint8_t  media_type;
        uint16_t sectors_per_fat_16;
        uint16_t sectors_per_track;
        uint16_t head_count;
        uint32_t hidden_sectors;
        uint32_t total_sectors_32;
        uint32_t sectors_per_fat_32;
        uint16_t flags;
        uint16_t version;
        uint32_t root_cluster;
        uint16_t fs_info_sector;
        uint16_t backup_boot_sector;
        uint8_t  reserved[12];
        uint8_t  drive_number;
        uint8_t  reserved1;
        uint8_t  boot_signature;
        uint32_t volume_id;
        char     volume_label[11];
        char     fs_type[8];
    };

    struct DirEntry {
        char     name[11];
        uint8_t  attributes;
        uint8_t  reserved;
        uint8_t  creation_time_tenths;
        uint16_t creation_time;
        uint16_t creation_date;
        uint16_t last_access_date;
        uint16_t first_cluster_high;
        uint16_t last_modification_time;
        uint16_t last_modification_date;
        uint16_t first_cluster_low;
        uint32_t file_size;
    };
    #pragma pack(pop)

    static BootSector    g_bootSector;
    static uint32_t      g_fatLba;
    static uint32_t      g_dataLba;
    static uint32_t      g_totalClusters;

    // Helpers
    static bool          ReadCluster(uint32_t cluster, uint8_t* buffer);
    static bool          WriteCluster(uint32_t cluster, const uint8_t* buffer);
    static uint32_t      GetNextCluster(uint32_t cluster);
    static bool          UpdateFatEntry(uint32_t cluster, uint32_t value);
    static uint32_t      FindFreeCluster();
    static DirEntry*     FindFileEntry(const char* name);
    static DirEntry*     FindFreeEntry();
    static void         FormatName(const char* input, char output[11]);
};