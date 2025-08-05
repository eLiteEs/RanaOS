#include "fat32.h"

FAT32::FAT32() : partition_start(0), mounted(false) {
    // Inicialización adicional si es necesaria
}

// Remove the CHECK_SECTOR_OP macro and replace with this function
static bool check_sector_op(bool success) {
    return success;
}

uint32_t FAT32::cluster_to_lba(uint32_t cluster) {
    if (cluster < 2) {
        // Invalid cluster number (0 and 1 are reserved)
        return 0;
    }
    return data_start + (cluster - 2) * bs.sectors_per_cluster;
}

bool FAT32::mount(uint32_t partition_lba) {
    partition_start = partition_lba;
    ATA::read_sectors(partition_lba, 1, &bs);
    
    // Verify signature and FS type
    if (bs.signature != 0xAA55 || strncmp(bs.fs_type, "FAT32   ", 8) != 0) {
        return false;
    }
    
    fat_start = partition_start + bs.reserved_sectors;
    data_start = fat_start + (bs.fat_count * bs.fat_size_32);
    current_dir_cluster = bs.root_cluster;
    mounted = true;
    
    return true;
}

bool FAT32::format(uint32_t total_sectors) {
    FAT32_BootSector new_bs = {0};
    
    // Initialize boot sector fields
    new_bs.jump[0] = 0xEB;
    new_bs.jump[1] = 0x58;
    new_bs.jump[2] = 0x90;
    memcpy(new_bs.oem, "MYOSFAT32", 8);
    new_bs.bytes_per_sector = 512;
    new_bs.sectors_per_cluster = 8;
    new_bs.reserved_sectors = 32;
    new_bs.fat_count = 2;
    new_bs.root_entries = 0;
    new_bs.total_sectors_16 = 0;
    new_bs.media_type = 0xF8;
    new_bs.fat_size_16 = 0;
    new_bs.sectors_per_track = 63;
    new_bs.head_count = 16;
    new_bs.hidden_sectors = partition_start;
    new_bs.total_sectors_32 = total_sectors;
    
    // Calculate FAT size
    uint32_t data_sectors = total_sectors - new_bs.reserved_sectors;
    uint32_t cluster_count = data_sectors / new_bs.sectors_per_cluster;
    new_bs.fat_size_32 = (cluster_count * 4 + 511) / 512;
    
    new_bs.ext_flags = 0;
    new_bs.fs_version = 0;
    new_bs.root_cluster = 2;
    new_bs.fs_info = 1;
    new_bs.backup_boot = 6;
    new_bs.signature = 0xAA55;
    memcpy(new_bs.fs_type, "FAT32   ", 8);
    
    // Write boot sector
    ATA::write_sectors(partition_start, 1, &new_bs);
    
    // Write FSInfo sector
    uint8_t fsinfo[512] = {0};
    fsinfo[0] = 0x52; fsinfo[1] = 0x52; fsinfo[2] = 0x61; fsinfo[3] = 0x41;
    *(uint32_t*)(fsinfo + 488) = 0x61417272;
    *(uint32_t*)(fsinfo + 492) = 0xFFFFFFFF;
    *(uint32_t*)(fsinfo + 496) = 0xFFFFFFFF;
    fsinfo[510] = 0x55; fsinfo[511] = 0xAA;
    
    ATA::write_sectors(partition_start + 1, 1, fsinfo);
    
    // Initialize FAT
    uint32_t fat_size = new_bs.fat_size_32;
    uint8_t* fat = new uint8_t[fat_size * 512];
    memset(fat, 0, fat_size * 512);
    
    // Mark first clusters
    *(uint32_t*)fat = 0x0FFFFFF8;
    *(uint32_t*)(fat + 4) = 0xFFFFFFFF;
    *(uint32_t*)(fat + 8) = 0x0FFFFFFF;
    
    // Write FAT copies
    ATA::write_sectors(fat_start, fat_size, fat);
    ATA::write_sectors(fat_start + fat_size, fat_size, fat);
    
    delete[] fat;
    
    // Initialize root directory
    uint8_t root_dir[512] = {0};
    FAT32_DirEntry* dot = (FAT32_DirEntry*)root_dir;
    memcpy(dot->name, ".       ", 11);
    dot->attributes = ATTR_DIRECTORY;
    dot->cluster_high = new_bs.root_cluster >> 16;
    dot->cluster_low = new_bs.root_cluster & 0xFFFF;
    
    FAT32_DirEntry* dotdot = dot + 1;
    memcpy(dotdot->name, "..      ", 11);
    dotdot->attributes = ATTR_DIRECTORY;
    
    uint32_t root_dir_sector = cluster_to_lba(new_bs.root_cluster);
    ATA::write_sectors(root_dir_sector, 1, root_dir);
    
    // Update internal state
    memcpy(&bs, &new_bs, sizeof(FAT32_BootSector));
    mounted = true;
    
    return true;
}

// Implement other functions with proper return values
FAT32_Result FAT32::fread(uint32_t file_handle, void* buffer, uint32_t size, uint32_t* bytes_read) {
    *bytes_read = 0;
    return FAT32_OK;
}

FAT32_Result FAT32::fwrite(uint32_t file_handle, const void* buffer, uint32_t size, uint32_t* bytes_written) {
    *bytes_written = 0;
    return FAT32_OK;
}

FAT32_Result FAT32::fclose(uint32_t file_handle) {
    return FAT32_OK;
}

FAT32_Result FAT32::opendir(const char* path, uint32_t* dir_handle) {
    return FAT32_OK;
}

FAT32_Result FAT32::readdir(uint32_t dir_handle, FAT32_DirEntry* entry) {
    return FAT32_OK;
}

FAT32_Result FAT32::closedir(uint32_t dir_handle) {
    return FAT32_OK;
}

FAT32_Result FAT32::mkdir(const char* path) {
    return FAT32_OK;
}

FAT32_Result FAT32::rename(const char* old_path, const char* new_path) {
    return FAT32_OK;
}

bool FAT32::find_file_in_dir(uint32_t dir_cluster, const char* name, FAT32_DirEntry* entry) {
    return false;
}

bool FAT32::is_valid_short_name(const char* name) {
    return false;
}

uint32_t FAT32::get_next_cluster(uint32_t cluster) {
    return 0;
}

bool FAT32::set_next_cluster(uint32_t cluster, uint32_t next_cluster) {
    return false;
}