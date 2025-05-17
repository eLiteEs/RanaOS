#include <stdint.h>

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Instala el handler de teclado y habilita IRQ1.
 * Llama a remap_pic(), set_idt_gate(0x21, ...), enable_irq(1) y sti().
 */
void keyboard_install();

/**
 * Devuelve el siguiente código ASCII (o negativo para flechas).
 * Espera (bloquea) hasta que haya una tecla.
 */
// char getKey();
/**
 * Handler de IRQ1, llamado desde ensamblador.
 */
void keyboard_handler();

char scancodeToAscii(uint8_t sc);

#ifdef __cplusplus
}
#endif

#endif // KEYBOARD_H

