#include "part_mgr.h"
#include "disk.h"
#include "string.h"

const char* get_partition_type_name(uint8_t type) {
    switch (type) {
        case PARTITION_EMPTY: return "Empty";
        case PARTITION_FAT12: return "FAT12";
        case PARTITION_FAT16: return "FAT16";
        case PARTITION_FAT32: return "FAT32";
        case PARTITION_FAT32_LBA: return "FAT32 (LBA)";
        case PARTITION_LINUX_SWAP: return "Linux Swap";
        case PARTITION_LINUX_NATIVE: return "Linux Native";
        default: return "Unknown";
    }
}

bool detect_disks(DiskInfo* disks, int max_count, int* count) {
    if (max_count < 1) return false;
    
    uint16_t id_data[256];
    ATA::identify_drive(id_data);
    
    // Fix strict-aliasing warning
    uint32_t total_sectors;
    memcpy(&total_sectors, &id_data[60], sizeof(uint32_t));
    
    DiskInfo* di = &disks[0];
    di->total_sectors = total_sectors;
    di->cylinders = id_data[1];
    di->heads = id_data[3];
    di->sectors_per_track = id_data[6];
    
    // Extraer modelo (40 caracteres, big-endian)
    for (int i = 0; i < 20; i++) {
        di->model[i*2] = id_data[27+i] >> 8;
        di->model[i*2+1] = id_data[27+i] & 0xFF;
    }
    di->model[40] = '\0';
    
    *count = 1;
    return true;
}
bool read_partitions(uint32_t disk_lba, PartitionInfo* partitions, int max_count, int* count) {
    MBR mbr;
    ATA::read_sectors(disk_lba, 1, &mbr);
    
    if (mbr.signature != 0xAA55) return false;
    
    int found = 0;
    for (int i = 0; i < 4 && found < max_count; i++) {
        if (mbr.partitions[i].type != PARTITION_EMPTY) {
            PartitionInfo* pi = &partitions[found++];
            pi->status = mbr.partitions[i].status;
            pi->type = mbr.partitions[i].type;
            pi->start_lba = mbr.partitions[i].lba_start;
            pi->sector_count = mbr.partitions[i].sector_count;
            
            const char* name = get_partition_type_name(pi->type);
            int j;
            for (j = 0; j < 31 && name[j]; j++) {
                pi->type_name[j] = name[j];
            }
            pi->type_name[j] = '\0';
        }
    }
    
    *count = found;
    return true;
}

bool create_partition(uint32_t disk_lba, int index, uint8_t type, uint32_t start_lba, uint32_t size_sectors) {
    if (index < 0 || index > 3) return false;
    
    MBR mbr;
    ATA::read_sectors(disk_lba, 1, &mbr);
    
    // Verificar solapamiento
    for (int i = 0; i < 4; i++) {
        if (mbr.partitions[i].type != PARTITION_EMPTY) {
            uint32_t end = mbr.partitions[i].lba_start + mbr.partitions[i].sector_count;
            if (start_lba < end && (start_lba + size_sectors) > mbr.partitions[i].lba_start) {
                return false; // Solapamiento
            }
        }
    }
    
    mbr.partitions[index].status = 0x80; // Activa
    mbr.partitions[index].type = type;
    mbr.partitions[index].lba_start = start_lba;
    mbr.partitions[index].sector_count = size_sectors;
    
    mbr.signature = 0xAA55;
    ATA::write_sectors(disk_lba, 1, &mbr);
    return true;
}

bool delete_partition(uint32_t disk_lba, int index) {
    if (index < 0 || index > 3) return false;
    
    MBR mbr;
    ATA::read_sectors(disk_lba, 1, &mbr);
    
    mbr.partitions[index].type = PARTITION_EMPTY;
    ATA::write_sectors(disk_lba, 1, &mbr);
    
    return true;
}

bool resize_partition(uint32_t disk_lba, int index, uint32_t new_size_sectors) {
    if (index < 0 || index > 3) return false;
    
    MBR mbr;
    ATA::read_sectors(disk_lba, 1, &mbr);
    
    if (mbr.partitions[index].type == PARTITION_EMPTY) return false;
    
    uint32_t new_end = mbr.partitions[index].lba_start + new_size_sectors;
    
    // Verificar solapamiento
    for (int i = 0; i < 4; i++) {
        if (i != index && mbr.partitions[i].type != PARTITION_EMPTY) {
            uint32_t other_start = mbr.partitions[i].lba_start;
            uint32_t other_end = other_start + mbr.partitions[i].sector_count;
            
            if (new_end > other_start && mbr.partitions[index].lba_start < other_end) {
                return false; // Solapamiento
            }
        }
    }
    
    mbr.partitions[index].sector_count = new_size_sectors;
    ATA::write_sectors(disk_lba, 1, &mbr);
    return true;
}

bool format_fat32(uint32_t partition_lba, uint32_t total_sectors) {
    // 1. Crear boot sector FAT32
    uint8_t boot_sector[512] = {0};
    
    // Configurar valores básicos
    boot_sector[0] = 0xEB; // Jump instruction
    boot_sector[1] = 0x58;
    boot_sector[2] = 0x90;
    memcpy(boot_sector + 3, "MYOSFAT32", 8); // OEM
    *((uint16_t*)(boot_sector + 11)) = 512;   // Bytes per sector
    boot_sector[13] = 8;                      // Sectors per cluster
    *((uint16_t*)(boot_sector + 14)) = 32;    // Reserved sectors
    boot_sector[16] = 2;                      // FAT copies
    *((uint16_t*)(boot_sector + 17)) = 0;     // Root entries (0 for FAT32)
    *((uint16_t*)(boot_sector + 19)) = 0;     // Total sectors (16-bit)
    boot_sector[21] = 0xF8;                   // Media descriptor
    *((uint16_t*)(boot_sector + 22)) = 0;     // FAT size (16-bit)
    *((uint16_t*)(boot_sector + 24)) = 63;    // Sectors per track
    *((uint16_t*)(boot_sector + 26)) = 16;    // Number of heads
    *((uint32_t*)(boot_sector + 28)) = 0;     // Hidden sectors
    *((uint32_t*)(boot_sector + 32)) = total_sectors; // Total sectors (32-bit)
    
    // Calcular tamaño FAT
    uint32_t data_sectors = total_sectors - 32;
    uint32_t cluster_count = data_sectors / 8;
    uint32_t fat_size = (cluster_count * 4 + 511) / 512;
    
    *((uint32_t*)(boot_sector + 36)) = fat_size; // FAT size (32-bit)
    
    boot_sector[40] = 0; // Ext flags
    boot_sector[41] = 0;
    *((uint16_t*)(boot_sector + 42)) = 0; // FS version
    *((uint32_t*)(boot_sector + 44)) = 2; // Root cluster
    *((uint16_t*)(boot_sector + 48)) = 1; // FS info sector
    *((uint16_t*)(boot_sector + 50)) = 6; // Backup boot sector
    memcpy(boot_sector + 54, "NO NAME    ", 11); // Volume label
    memcpy(boot_sector + 71, "FAT32   ", 8);     // FS type
    
    // Signature
    boot_sector[510] = 0x55;
    boot_sector[511] = 0xAA;
    
    // Escribir boot sector
    ATA::write_sectors(partition_lba, 1, boot_sector);
    
    // 2. Escribir FSInfo
    uint8_t fsinfo[512] = {0};
    fsinfo[0] = 0x52; // Signature 1
    fsinfo[1] = 0x52;
    fsinfo[2] = 0x61;
    fsinfo[3] = 0x41;
    *((uint32_t*)(fsinfo + 488)) = 0x61417272; // Signature 2
    *((uint32_t*)(fsinfo + 492)) = 0xFFFFFFFF; // Free cluster count
    *((uint32_t*)(fsinfo + 496)) = 0xFFFFFFFF; // Next free cluster
    fsinfo[510] = 0x55;
    fsinfo[511] = 0xAA;
    ATA::write_sectors(partition_lba + 1, 1, fsinfo);
    
    // 3. Inicializar FATs
    uint32_t fat_bytes = fat_size * 512;
    uint8_t* fat = new uint8_t[fat_bytes];
    memset(fat, 0, fat_bytes);
    
    // Primeros clusters reservados
    *((uint32_t*)(fat)) = 0x0FFFFFF8; // Media descriptor
    *((uint32_t*)(fat + 4)) = 0xFFFFFFFF; // Estado limpio
    *((uint32_t*)(fat + 8)) = 0x0FFFFFFF; // EOF para cluster 2 (raíz)
    
    // Escribir ambas copias de FAT
    ATA::write_sectors(partition_lba + 32, fat_size, fat);
    ATA::write_sectors(partition_lba + 32 + fat_size, fat_size, fat);
    
    delete[] fat;
    
    // 4. Directorio raíz
    uint8_t root_dir[512] = {0};
    
    // Entrada para "."
    root_dir[0] = '.'; // Nombre
    root_dir[11] = 0x10; // Atributo: directorio
    *((uint16_t*)(root_dir + 20)) = 0; // Cluster alto
    *((uint16_t*)(root_dir + 26)) = 2; // Cluster bajo
    
    // Entrada para ".."
    root_dir[32] = '.'; // Nombre
    root_dir[33] = '.';
    root_dir[43] = 0x10; // Atributo: directorio
    
    ATA::write_sectors(partition_lba + 32 + fat_size * 2, 1, root_dir);
    
    return true;
}