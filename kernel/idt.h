#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Estructura para una entrada de la IDT (interrupt gate)
struct IDTEntry {
    uint16_t offset_low;   // bits 0–15 de la dirección del handler
    uint16_t selector;     // selector de segmento de código (p.ej. 0x08)
    uint8_t  zero;         // debe ser cero
    uint8_t  flags;        // tipo y atributos (p.e. 0x8E)
    uint16_t offset_high;  // bits 16–31 de la dirección
} __attribute__((packed));

// Puntero que usa la instrucción lidt
struct IDTPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Carga la IDT usando lidt(ptr)
extern "C" void load_idt(uint32_t idt_ptr_address);

/**
 * Instalación de una entrada de interrupción en el vector 'num'.
 *  - num: número de vector (0–255)
 *  - base: dirección de la función handler
 *  - sel: selector de segmento de código (0x08)
 *  - flags: atributos (0x8E para interrupt gate)
 */
void set_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif // IDT_H

