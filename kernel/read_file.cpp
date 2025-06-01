#include "fatnenuphar.h"
#include "disk.h"
#include <stdint.h>

extern DiskInfo detected_disks[4];
extern int detected_disk_count;

// Find disk by letter
DiskInfo* find_disk_by_letter(char letter) {
    for (int i = 0; i < detected_disk_count; ++i) {
        if (detected_disks[i].letter == letter)
            return &detected_disks[i];
    }
    return 0;
}

// Read full disk into memory
bool read_entire_disk(const DiskInfo& disk, void* out_buffer, uint32_t size_in_bytes) {
    uint32_t total_sectors = size_in_bytes / 512;
    for (uint32_t i = 0; i < total_sectors; ++i) {
        if (!read_sector(disk, i, (uint8_t*)out_buffer + i * 512))
            return false;
    }
    return true;
}

// Main function
char** load_file_content(char disk_letter, const char* filename) {
    static uint8_t disk_image[128 * 1024];            // Entire disk image (128 KB max)
    static char    file_buffer[64 * 1024];            // Max 64 KB file
    static char*   result_array[1] = { file_buffer }; // char**

    DiskInfo* disk = find_disk_by_letter(disk_letter);
    if (!disk) return 0;

    if (!read_entire_disk(*disk, disk_image, sizeof(disk_image)))
        return 0;

    fn_init(disk_image + FN_DIR_SECTORS * FN_SECTOR_SIZE, sizeof(disk_image) - FN_DIR_SECTORS * FN_SECTOR_SIZE);

    int read_bytes = fn_read_file(filename, file_buffer, sizeof(file_buffer) - 1);
    if (read_bytes <= 0)
        return 0;

    file_buffer[read_bytes] = '\0';
    return result_array;
}
