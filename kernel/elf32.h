#pragma once
#include <stdint.h>

// Identificadores de tipo de segmento
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6

// ELF Header
typedef struct {
    unsigned char e_ident[16]; // Magia, clase, endianess, versión...
    uint16_t e_type;            // Tipo de ELF
    uint16_t e_machine;         // Arquitectura
    uint32_t e_version;         // Versión
    uint32_t e_entry;           // Dirección de entrada
    uint32_t e_phoff;           // Offset a tabla de programas
    uint32_t e_shoff;           // Offset a tabla de secciones
    uint32_t e_flags;           // Flags específicos de arquitectura
    uint16_t e_ehsize;          // Tamaño del ELF header
    uint16_t e_phentsize;       // Tamaño de cada entrada de tabla de programas
    uint16_t e_phnum;           // Número de entradas de tabla de programas
    uint16_t e_shentsize;       // Tamaño de cada entrada de sección
    uint16_t e_shnum;           // Número de entradas de sección
    uint16_t e_shstrndx;        // Índice de sección con nombres
} Elf32_Ehdr;

// Program Header (Segmentos)
typedef struct {
    uint32_t p_type;   // Tipo de segmento (PT_LOAD, PT_NULL, etc.)
    uint32_t p_offset; // Offset del segmento en el archivo ELF
    uint32_t p_vaddr;  // Dirección virtual donde cargar el segmento
    uint32_t p_paddr;  // Dirección física (no usado aquí)
    uint32_t p_filesz; // Tamaño del segmento en el archivo
    uint32_t p_memsz;  // Tamaño del segmento en memoria (BSS incluido)
    uint32_t p_flags;  // Flags de segmento (R/W/X)
    uint32_t p_align;  // Alineación
} Elf32_Phdr;

