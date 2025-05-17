#include "idt.h"

// Array de 256 entradas de la IDT
static IDTEntry idt[256];
static IDTPtr   idt_ptr;

// Stub en ensamblador para cargar la IDT (lidt).  
// Recibe en la pila la dirección de idt_ptr.
extern "C" {
__asm__ (
".globl load_idt           \n"
"load_idt:                 \n"
"  movl 4(%esp), %eax      \n"  // obtener argumento (puntero a IDTPtr)
"  lidt (%eax)             \n"
"  ret                      \n"
);
}

void set_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].selector    = sel;
    idt[num].zero        = 0;
    idt[num].flags       = flags;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
}

static void init_idt() {
    // Prepara el puntero
    idt_ptr.limit = sizeof(IDTEntry)*256 - 1;
    idt_ptr.base  = (uint32_t)&idt;

    // Inicializa todas las entradas a cero
    for (int i = 0; i < 256; ++i) {
        set_idt_gate(i, 0, 0, 0);
    }

    // Carga la IDT
    load_idt((uint32_t)&idt_ptr);
}

// Constructor estático: se ejecuta antes de main/kmain
struct IDTInitializer {
    IDTInitializer() { init_idt(); }
} _idt_init;

