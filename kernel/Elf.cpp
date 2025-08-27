#include "Elf.h"
#include "Debug.h"
#include "string.h"

extern "C" {
    #include "elf32.h" // Estructuras ELF32: Elf32_Ehdr, Elf32_Phdr, PT_LOAD...
}

namespace Elf {

    // Puntero global a syscalls accesible desde el ELF
    Syscalls* sys = nullptr;

    bool execute(const char* file, Syscalls* s) {
        sys = s;

        const Elf32_Ehdr* ehdr = reinterpret_cast<const Elf32_Ehdr*>(file);

        // Validar ELF básico
        if (!(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 'E' &&
              ehdr->e_ident[2] == 'L' && ehdr->e_ident[3] == 'F')) {
            Debug::Print("Error: archivo no es ELF.\n");
            return false;
        }

        const Elf32_Phdr* phdr = reinterpret_cast<const Elf32_Phdr*>(file + ehdr->e_phoff);

        for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
            if (phdr[i].p_type != PT_LOAD) continue;

            uint8_t* segment = reinterpret_cast<uint8_t*>(phdr[i].p_vaddr);
            const uint8_t* src = reinterpret_cast<const uint8_t*>(file + phdr[i].p_offset);

            // Copiar datos del ELF al segmento
            for (uint32_t b = 0; b < phdr[i].p_filesz; b++) {
                segment[b] = src[b];
            }

            // BSS: inicializar a cero lo que sobra
            for (uint32_t b = phdr[i].p_filesz; b < phdr[i].p_memsz; b++) {
                segment[b] = 0;
            }
        }

        // Llamar a la función de entrada
        using Entry = void(*)(Syscalls*);
        Entry entry = reinterpret_cast<Entry>(ehdr->e_entry);
        entry(sys);

        return true;
    }

}

