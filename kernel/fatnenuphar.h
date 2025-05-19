// fatnenuphar.h
#pragma once
#include <stdint.h>

#define FN_MAX_NAME      32
#define FN_MAX_FILES     128
#define FN_SECTOR_SIZE   512
#define FN_DIR_SECTORS   4
#define FN_DIR_ENTRIES   (FN_DIR_SECTORS * (FN_SECTOR_SIZE / sizeof(FN_FileEntry)))

struct FN_FileEntry {
    char     filename[FN_MAX_NAME];
    uint32_t start_sector;
    uint32_t sector_count;
    uint32_t file_size;
    uint8_t  used;
};

// Inicializa la imagen cargada (debe apuntar a todo el disco en RAM)
void fn_init(void* disk_image, uint32_t disk_size_bytes);

// Crea un archivo nuevo; retorna 0 en éxito, negativo en error
int fn_create_file(const char* name, const void* data, uint32_t size);

int fn_list_files(FN_FileEntry* entries, int max_entries);
int fn_read_file(const char* name, void* buffer, uint32_t max_size);
