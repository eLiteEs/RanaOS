// kernel_panic.cpp
#include "Console.h"
#include "idt.h"

extern "C" void kernel_panic(const char* msg, Registers* regs) {
    // Deshabilitar interrupciones
    asm volatile("cli");

    // Color: fondo rojo, texto blanco
    const uint8_t color = 0x4F;

    Console::clearScreen();

    // Mostrar mensaje de panic
    Console::println("KERNEL PANIC");

    // Mostrar mensaje específico
    Console::println("Message: ", msg);

    // Mostrar número de excepción
    

    // Detener el sistema
    while(1) asm volatile("hlt");
}