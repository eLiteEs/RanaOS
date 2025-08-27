#pragma once
#include <stdint.h>

// Tabla de funciones que los ELF pueden usar para llamar al kernel
struct Syscalls {
    // Consola de texto
    void (*console_write)(const char* str);     // Imprime sin salto de línea
    void (*console_println)(const char* str);   // Imprime con salto de línea

    // Depuración
    void (*debug_print)(const char* str);       // Imprime en modo debug
    void (*debug_print_dec)(int value);         // Imprime un entero decimal

    // Gráficos
    void (*put_pixel)(uint32_t x, uint32_t y, uint32_t color);
    uint32_t (*get_pixel)(uint32_t x, uint32_t y);

    // Puedes añadir más funciones según necesites
};

