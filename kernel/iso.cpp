#include <stdint.h>
#include <string.h>
#include "disk.hpp"
#include "Console.h"

namespace ISO9660 {

#pragma pack(push, 1)
// Primary Volume Descriptor (PVD)
struct PVD {
    uint8_t type;               // 0x01 for PVD
    char standard_id[5];        // "CD001"
    uint8_t version;            // 0x01
    uint8_t unused1;
    char system_id[32];
    char volume_id[32];
    uint8_t unused2[8];
    uint32_t volume_space_size_le; // Little-endian
    uint32_t volume_space_size_be; // Big-endian
    uint8_t unused3[32];
    uint16_t volume_set_size_le;
    uint16_t volume_set_size_be;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint16_t logical_block_size_le; // Typically 2048
    uint16_t logical_block_size_be;
    uint32_t path_table_size_le;
    uint32_t path_table_size_be;
    uint32_t location_of_path_table_le;
    uint32_t location_of_path_table_be;
    uint32_t location_of_optional_path_table_le;
    uint32_t location_of_optional_path_table_be;
    struct DirectoryEntry {
        uint8_t length;
        uint8_t extended_attribute_length;
        uint32_t extent_location_le;
        uint32_t extent_location_be;
        uint32_t data_length_le;
        uint32_t data_length_be;
        uint8_t recording_date_time[7];
        uint8_t flags;
        uint8_t file_unit_size;
        uint8_t interleave_gap_size;
        uint16_t volume_sequence_number_le;
        uint16_t volume_sequence_number_be;
        uint8_t file_id_length;
        char file_id[];
    } root_directory;
};
#pragma pack(pop)

// Directory Entry
struct DirectoryEntry {
    uint8_t length;
    uint8_t extended_attribute_length;
    uint32_t extent_location;   // LBA of the file
    uint32_t data_length;       // Size of the file in bytes
    uint8_t flags;
    char name[256];             // File name (null-terminated)
};

// Function to convert ISO9660 date/time to a readable format
void format_date_time(const uint8_t* iso_time, char* buffer) {
    // Format: YYYY-MM-DD HH:MM:SS (GMT)
    snprintf(buffer, 20, "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + iso_time[0], iso_time[1], iso_time[2],
        iso_time[3], iso_time[4], iso_time[5]);
}

// Read a directory entry from a buffer
bool read_directory_entry(const uint8_t* data, DirectoryEntry* entry) {
    if (data[0] == 0) return false; // No more entries
    
    // Basic info
    entry->length = data[0];
    entry->extended_attribute_length = data[1];
    
    // File location and size (little-endian)
    entry->extent_location = 
        (data[2]) | (data[3] << 8) | (data[4] << 16) | (data[5] << 24);
    entry->data_length = 
        (data[10]) | (data[11] << 8) | (data[12] << 16) | (data[13] << 24);
    
    // Flags
    entry->flags = data[25];
    
    // File name length
    uint8_t file_id_length = data[32];
    
    // Copy file name
    if (file_id_length > 0) {
        // Skip version suffix (;1)
        uint8_t name_length = file_id_length;
        if (data[32 + file_id_length - 2] == ';' && 
            isdigit(data[32 + file_id_length - 1])) {
            name_length -= 2;
        }
        
        memcpy(entry->name, &data[33], name_length);
        entry->name[name_length] = '\0';
    } else {
        entry->name[0] = '\0';
    }
    
    return true;
}

// Find a file in the ISO9660 filesystem
bool find_file(const char* path, DirectoryEntry* entry) {
    // Read the Primary Volume Descriptor (sector 16)
    PVD pvd;
    if (!Disk::ReadSector(16, reinterpret_cast<uint8_t*>(&pvd))) {
        Console::println("Error reading PVD");
        return false;
    }
    
    // Verify PVD signature
    if (memcmp(pvd.standard_id, "CD001", 5) != 0) {
        Console::println("Invalid PVD signature");
        return false;
    }
    
    // Get root directory location and size
    uint32_t root_dir_lba = pvd.root_directory.extent_location_le;
    uint32_t root_dir_size = pvd.root_directory.data_length_le;
    
    // Calculate number of sectors for root directory
    uint32_t root_dir_sectors = (root_dir_size + 511) / 512;
    
    // Read root directory
    uint8_t* root_dir_data = new uint8_t[root_dir_sectors * 512];
    for (uint32_t i = 0; i < root_dir_sectors; i++) {
        if (!Disk::ReadSector(root_dir_lba + i, root_dir_data + i * 512)) {
            delete[] root_dir_data;
            Console::println("Error reading root directory");
            return false;
        }
    }
    
    // Parse root directory entries
    bool found = false;
    uint32_t offset = 0;
    
    while (offset < root_dir_size) {
        DirectoryEntry current_entry;
        if (!read_directory_entry(root_dir_data + offset, &current_entry)) {
            break; // End of directory
        }
        
        // Skip empty entries
        if (current_entry.length == 0) {
            offset++;
            continue;
        }
        
        // Check if this is the file we're looking for
        if (strcmp(current_entry.name, path) == 0) {
            *entry = current_entry;
            found = true;
            break;
        }
        
        // Move to next entry
        offset += current_entry.length;
    }
    
    delete[] root_dir_data;
    return found;
}

// Read a file from the ISO
bool read_file(const char* path, uint8_t* buffer, uint32_t max_size) {
    DirectoryEntry entry;
    if (!find_file(path, &entry)) {
        Console::write("File not found: ");
        Console::println(path);
        return false;
    }
    
    // Check if it's a directory
    if (entry.flags & 0x02) {
        Console::write(path);
        Console::println(" is a directory");
        return false;
    }
    
    // Check if file is too big for buffer
    if (entry.data_length > max_size) {
        Console::write("File too big: ");
        Console::write(entry.data_length);
        Console::write(" > ");
        Console::println(max_size);
        return false;
    }
    
    // Calculate number of sectors to read
    uint32_t sectors = (entry.data_length + 511) / 512;
    
    // Read file content
    for (uint32_t i = 0; i < sectors; i++) {
        if (!Disk::ReadSector(entry.extent_location + i, buffer + i * 512)) {
            Console::println("Error reading file content");
            return false;
        }
    }
    
    Console::write("Successfully read file: ");
    Console::println(path);
    Console::write("Size: ");
    Console::write(entry.data_length);
    Console::println(" bytes");
    
    return true;
}

// List root directory contents
void list_root_directory() {
    // Read the Primary Volume Descriptor (sector 16)
    PVD pvd;
    if (!Disk::ReadSector(16, reinterpret_cast<uint8_t*>(&pvd))) {
        Console::println("Error reading PVD");
        return;
    }
    
    // Get root directory location and size
    uint32_t root_dir_lba = pvd.root_directory.extent_location_le;
    uint32_t root_dir_size = pvd.root_directory.data_length_le;
    
    // Calculate number of sectors for root directory
    uint32_t root_dir_sectors = (root_dir_size + 511) / 512;
    
    // Read root directory
    uint8_t* root_dir_data = new uint8_t[root_dir_sectors * 512];
    for (uint32_t i = 0; i < root_dir_sectors; i++) {
        if (!Disk::ReadSector(root_dir_lba + i, root_dir_data + i * 512)) {
            delete[] root_dir_data;
            Console::println("Error reading root directory");
            return;
        }
    }
    
    // Parse and list root directory entries
    Console::println("Root directory contents:");
    Console::println("------------------------");
    
    uint32_t offset = 0;
    uint32_t file_count = 0;
    uint32_t dir_count = 0;
    
    while (offset < root_dir_size) {
        DirectoryEntry entry;
        if (!read_directory_entry(root_dir_data + offset, &entry)) {
            break; // End of directory
        }
        
        // Skip empty entries
        if (entry.length == 0) {
            offset++;
            continue;
        }
        
        // Print entry info
        char date_buffer[20];
        format_date_time(entry.recording_date_time, date_buffer);
        
        Console::write(entry.flags & 0x02 ? "D " : "F ");
        Console::write(entry.data_length);
        Console::write(" bytes\t");
        Console::write(date_buffer);
        Console::write("\t");
        Console::println(entry.name);
        
        if (entry.flags & 0x02) dir_count++;
        else file_count++;
        
        // Move to next entry
        offset += entry.length;
    }
    
    delete[] root_dir_data;
    
    Console::println("------------------------");
    Console::write("Total: ");
    Console::write(file_count);
    Console::write(" files, ");
    Console::write(dir_count);
    Console::println(" directories");
}

} // namespace ISO9660