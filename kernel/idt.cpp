// idt.cpp - Implementación de la IDT
#include "idt.h"
#include "io.h"
#include <stddef.h>

extern "C" uint32_t isr_stub_table[];

// Variables estáticas
static IDTEntry idt[256];
static IDTPtr idt_ptr;
static ISRHandler handlers[256];

// Manejador genérico de interrupciones (definido en exceptions.asm)
extern "C" void isr_handler(Registers* regs) {
    if (handlers[regs->int_no]) {
        handlers[regs->int_no](regs);
    } else {
        // Llamar al kernel panic si no hay manejador registrado
        asm volatile("cli");
        while(1);
    }
}

// Registrar un manejador de interrupción
void register_interrupt_handler(uint8_t n, ISRHandler handler) {
    handlers[n] = handler;
}

// Inicializar la IDT
extern "C" void idt_init() {
    idt_ptr.limit = sizeof(IDTEntry) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Inicializar todas las entradas
    for (int i = 0; i < 256; i++) {
        handlers[i] = nullptr;
    }

    
    // Cargar la IDT
    asm volatile("lidt %0" : : "m"(idt_ptr));
}

// Manejadores básicos para excepciones
static void exception_handler(Registers* regs) {
    // Implementación básica que llama a kernel_panic
    static const char* messages[32] = {
        "Divide By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",
        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
    };
    
}

// Registra los manejadores de excepciones básicos
void initialize_exception_handlers() {
    for (int i = 0; i < 32; i++) {
        register_interrupt_handler(i, exception_handler);
    }
}

extern "C" void irq_handler(Registers* regs) {
    // Enviar EOI (End Of Interrupt) al PIC
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20); // EOI al PIC esclavo
    }
    outb(0x20, 0x20); // EOI al PIC maestro

    // Llamar al manejador registrado si existe
    if (handlers[regs->int_no]) {
        handlers[regs->int_no](regs);
    }
}


void idt_set_gate(uint8_t num, uint32_t handler, uint8_t flags) {
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = 0x08;  // Segmento de código del kernel
    idt[num].zero = 0;
    idt[num].type_attr = flags;
    idt[num].offset_high = (handler >> 16) & 0xFFFF;
}

extern "C" void setup_idt() {
    idt_ptr.limit = sizeof(IDTEntry) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Cargar la IDT
    asm volatile("lidt %0" : : "m"(idt_ptr));
}
