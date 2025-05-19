// fatnenuphar.cpp
#include "fatnenuphar.h"
#include <stdint.h>

// ─── Rutinas freestanding ────────────────────────────────────────────────────

extern "C" inline void* fn_memcpy(void* dest, const void* src, uint32_t n) {
    auto d = (uint8_t*)dest;
    auto s = (const uint8_t*)src;
    for (uint32_t i = 0; i < n; ++i) d[i] = s[i];
    return dest;
}

extern "C" inline void* fn_memset(void* dest, int c, uint32_t n) {
    auto d = (uint8_t*)dest;
    for (uint32_t i = 0; i < n; ++i) d[i] = (uint8_t)c;
    return dest;
}

extern "C" inline int fn_strncmp(const char* a, const char* b, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) {
        uint8_t ca = (uint8_t)a[i], cb = (uint8_t)b[i];
        if (ca != cb || ca == '\0' || cb == '\0')
            return ca - cb;
    }
    return 0;
}

static void fn_strncpy(char* dest, const char* src, uint32_t n) {
    uint32_t i = 0;
    for (; i + 1 < n && src[i]; ++i)
        dest[i] = src[i];
    for (; i < n; ++i)
        dest[i] = '\0';
}

// ─── Datos globales del FS ────────────────────────────────────────────────────

static uint8_t*      g_disk      = nullptr;
static uint32_t      g_disk_size = 0;
static FN_FileEntry* g_dir       = nullptr;

// ─── API pública ──────────────────────────────────────────────────────────────

void fn_init(void* disk_image, uint32_t disk_size_bytes) {
    g_disk      = (uint8_t*)disk_image;
    g_disk_size = disk_size_bytes;
    g_dir       = (FN_FileEntry*)g_disk;
}

// Máximo número de sectores que soportamos (128 KB / 512 = 256)
#ifndef FN_MAX_SECTORS
#define FN_MAX_SECTORS (128 * 1024 / FN_SECTOR_SIZE)
#endif

int fn_create_file(const char* name, const void* data, uint32_t size) {
    if (!g_dir || size == 0 || size > FN_SECTOR_SIZE * (FN_MAX_SECTORS - FN_DIR_SECTORS))
        return -1;

    // ¿Ya existe?
    for (uint32_t i = 0; i < FN_DIR_ENTRIES; ++i) {
        if (g_dir[i].used && fn_strncmp(g_dir[i].filename, name, FN_MAX_NAME) == 0)
            return -2;
    }

    // Buscar entrada libre
    int idx = -1;
    for (uint32_t i = 0; i < FN_DIR_ENTRIES; ++i) {
        if (!g_dir[i].used) { idx = i; break; }
    }
    if (idx < 0) return -3;

    // Calcular sectores necesarios
    uint32_t needed = (size + FN_SECTOR_SIZE - 1) / FN_SECTOR_SIZE;
    uint32_t totalS = g_disk_size / FN_SECTOR_SIZE;

    // Bitmap estático
    bool used[FN_MAX_SECTORS] = { false };

    // Marcar sectores de directorio
    for (uint32_t s = 0; s < FN_DIR_SECTORS && s < FN_MAX_SECTORS; ++s)
        used[s] = true;
    // Marcar sectores de archivos existentes
    for (uint32_t i = 0; i < FN_DIR_ENTRIES; ++i) {
        if (!g_dir[i].used) continue;
        uint32_t st = g_dir[i].start_sector;
        for (uint32_t s = 0; s < g_dir[i].sector_count; ++s) {
            if (st + s < FN_MAX_SECTORS)
                used[st + s] = true;
        }
    }

    // Buscar bloque contiguo
    int start = -1, run = 0;
    for (uint32_t s = FN_DIR_SECTORS; s < totalS; ++s) {
        if (!used[s]) {
            run++;
            if ((uint32_t)run == needed) {
                start = s + 1 - needed;
                break;
            }
        } else {
            run = 0;
        }
    }
    if (start < 0) return -4;

    // Rellenar la entrada
    FN_FileEntry& e = g_dir[idx];
    fn_strncpy(e.filename, name, FN_MAX_NAME);
    e.start_sector = start;
    e.sector_count = needed;
    e.file_size    = size;
    e.used         = 1;

    // Copiar datos
    fn_memcpy(g_disk + start * FN_SECTOR_SIZE, data, size);
    return 0;
}

int fn_read_file(const char* name, void* buffer, uint32_t max_size) {
    if (!g_dir || !buffer) return -1;
    for (uint32_t i = 0; i < FN_DIR_ENTRIES; ++i) {
        if (g_dir[i].used && fn_strncmp(g_dir[i].filename, name, FN_MAX_NAME) == 0) {
            uint32_t tocopy = (g_dir[i].file_size < max_size)
                                  ? g_dir[i].file_size
                                  : max_size;
            fn_memcpy(buffer, g_disk + g_dir[i].start_sector * FN_SECTOR_SIZE, tocopy);
            return (int)tocopy;
        }
    }
    return -2;
}

int fn_write_file(const char* name, const void* data, uint32_t size) {
    if (!g_dir || !data) return -1;
    for (uint32_t i = 0; i < FN_DIR_ENTRIES; ++i) {
        if (g_dir[i].used && fn_strncmp(g_dir[i].filename, name, FN_MAX_NAME) == 0) {
            uint32_t needed = (size + FN_SECTOR_SIZE - 1) / FN_SECTOR_SIZE;
            if (needed > g_dir[i].sector_count) return -2;
            g_dir[i].file_size = size;
            fn_memcpy(g_disk + g_dir[i].start_sector * FN_SECTOR_SIZE, data, size);
            return 0;
        }
    }
    return -3;
}

int fn_list_files(FN_FileEntry* entries, int max_entries) {
    if (!g_dir || !entries || max_entries <= 0) return 0;
    int count = 0;
    for (uint32_t i = 0; i < FN_DIR_ENTRIES && count < max_entries; ++i) {
        if (g_dir[i].used) {
            entries[count++] = g_dir[i];
        }
    }
    return count;
}

