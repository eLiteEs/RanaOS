#pragma once
#include <stdint.h>
#include "disk.h"
#include "string.h"

// Estructura del Boot Sector FAT32
struct FAT32_BootSector {
    uint8_t jump[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed));

// Estructura de entrada de directorio
struct FAT32_DirEntry {
    char name[11];
    uint8_t attributes;
    uint8_t nt_reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t mod_time;
    uint16_t mod_date;
    uint16_t cluster_low;
    uint32_t file_size;
} __attribute__((packed));

// Estructura para entrada de directorio largo (LFN)
struct FAT32_LongDirEntry {
    uint8_t order;
    uint16_t name1[5];
    uint8_t attributes;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t first_cluster;
    uint16_t name3[2];
} __attribute__((packed));

// Atributos de archivo
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

// Resultados de operaciones
enum FAT32_Result {
    FAT32_OK,
    FAT32_DISK_ERR,
    FAT32_NOT_READY,
    FAT32_NO_FILE,
    FAT32_NO_PATH,
    FAT32_INVALID_NAME,
    FAT32_DENIED,
    FAT32_EXIST,
    FAT32_INVALID_OBJECT,
    FAT32_WRITE_PROTECTED,
    FAT32_INVALID_DRIVE,
    FAT32_NOT_ENABLED,
    FAT32_NO_FILESYSTEM,
    FAT32_TIMEOUT,
    FAT32_LOCKED,
    FAT32_NOT_ENOUGH_CORE,
    FAT32_TOO_MANY_OPEN_FILES,
    FAT32_INVALID_PARAMETER
};

class FAT32 {
private:
    uint32_t partition_start;
    FAT32_BootSector bs;
    uint32_t fat_start;
    uint32_t data_start;
    uint32_t current_dir_cluster;
    bool mounted;

    // Funciones internas
    uint32_t cluster_to_lba(uint32_t cluster);
    uint32_t get_next_cluster(uint32_t cluster);
    bool set_next_cluster(uint32_t cluster, uint32_t next_cluster);
    uint32_t find_free_cluster();
    bool allocate_cluster_chain(uint32_t first_cluster, uint32_t count);
    bool free_cluster_chain(uint32_t cluster);
    uint32_t get_cluster_count(uint32_t cluster);
    bool read_cluster(uint32_t cluster, void* buffer);
    bool write_cluster(uint32_t cluster, const void* buffer);
    bool find_file_in_dir(uint32_t dir_cluster, const char* name, FAT32_DirEntry* entry);
    bool add_dir_entry(uint32_t dir_cluster, const FAT32_DirEntry* entry);
    bool delete_dir_entry(uint32_t dir_cluster, const char* name);
    bool update_dir_entry(uint32_t dir_cluster, const char* name, const FAT32_DirEntry* entry);
    bool create_short_name(const char* long_name, char* short_name);
    bool create_long_name_entries(const char* long_name, FAT32_LongDirEntry* entries, uint8_t* count);
    bool read_dir_entries(uint32_t dir_cluster, FAT32_DirEntry* entries, uint32_t max_entries, uint32_t* count);
    bool is_valid_short_name(const char* name);

public:
    FAT32();
    
    // Operaciones del sistema de archivos
    bool mount(uint32_t partition_lba);
    bool unmount();
    bool format(uint32_t total_sectors);
    
    // Operaciones con archivos
    FAT32_Result fopen(const char* path, uint8_t mode, uint32_t* file_handle);
    FAT32_Result fclose(uint32_t file_handle);
    FAT32_Result fread(uint32_t file_handle, void* buffer, uint32_t size, uint32_t* bytes_read);
    FAT32_Result fwrite(uint32_t file_handle, const void* buffer, uint32_t size, uint32_t* bytes_written);
    FAT32_Result fseek(uint32_t file_handle, uint32_t offset);
    FAT32_Result ftruncate(uint32_t file_handle, uint32_t size);
    FAT32_Result fstat(uint32_t file_handle, FAT32_DirEntry* entry);
    
    // Operaciones con directorios
    FAT32_Result opendir(const char* path, uint32_t* dir_handle);
    FAT32_Result readdir(uint32_t dir_handle, FAT32_DirEntry* entry);
    FAT32_Result closedir(uint32_t dir_handle);
    FAT32_Result mkdir(const char* path);
    FAT32_Result rmdir(const char* path);
    FAT32_Result chdir(const char* path);
    
    // Operaciones con archivos/directorios
    FAT32_Result stat(const char* path, FAT32_DirEntry* entry);
    FAT32_Result unlink(const char* path);
    FAT32_Result rename(const char* old_path, const char* new_path);
    
    // Operaciones misceláneas
    FAT32_Result getfree(uint32_t* total_clusters, uint32_t* free_clusters);
    FAT32_Result getlabel(char* label);
    FAT32_Result setlabel(const char* label);
    FAT32_Result sync();
    
    // Funciones utilitarias
    static bool is_valid_path(const char* path);
    static void to_short_name(const char* long_name, char* short_name);
    static void to_long_name(const char* short_name, char* long_name);
};