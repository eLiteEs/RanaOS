#pragma once
#include <stdint.h>
#include "Syscalls.h"

namespace Elf {

    // Ejecuta un ELF de 32 bits pasado como puntero a memoria
    // file: puntero al ELF cargado en memoria
    // sys: puntero a tabla de syscalls del kernel
    // devuelve true si se cargó correctamente
    bool execute(const char* file, Syscalls* sys);

}

