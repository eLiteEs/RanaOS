// idt.h - Definiciones para la Interrupt Descriptor Table
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Estructura de una entrada IDT
struct IDTEntry {
    uint16_t offset_low;    // bits 0-15 del offset
    uint16_t selector;      // selector de segmento de código
    uint8_t zero;           // siempre 0
    uint8_t type_attr;      // atributos (P, DPL, tipo)
    uint16_t offset_high;   // bits 16-31 del offset
} __attribute__((packed));

// Estructura del puntero IDT para LIDT
struct IDTPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Registros guardados durante una interrupción
struct Registers {
    uint32_t ds;                 // Segmento de datos
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Registros pusha
    uint32_t int_no, err_code;   // Número de interrupción y código de error
    uint32_t eip, cs, eflags, useresp, ss; // Automáticamente pusheados por el CPU
};

// Tipo para manejadores de interrupción
typedef void (*ISRHandler)(Registers*);

// Funciones públicas
extern "C" {
    void idt_init();
    void idt_set_gate(uint8_t num, uint32_t handler, uint8_t flags);
    void register_interrupt_handler(uint8_t n, ISRHandler handler);

    extern uint32_t isr_stub_table[];  // Declaración de la tabla de stubs
}

#endif // IDT_H