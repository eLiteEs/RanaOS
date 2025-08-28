#include "Syscalls.h"

extern "C" void _start(Syscalls* sys) {
    sys->console_write("Hola desde ELF!");
    sys->console_println(" Funciona con syscalls.");

    sys->console_println("fuee");
}

