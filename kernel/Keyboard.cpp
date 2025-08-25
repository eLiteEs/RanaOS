#include "Keyboard.h"

#include "Console.h"
#include "io.h"

int Keyboard::keyboard_key_available() {
    return inb(0x64) & 1;
}

uint8_t Keyboard::keyboard_read_scancode() {
    return inb(0x60);
}

// Funcion para comprobar si c fue pulsada
bool Keyboard::was_c_pressed() {
    if (!keyboard_key_available())
        return false;

    uint8_t sc = keyboard_read_scancode();

    // Ignora tecla liberada (bit 7 = 1)
    if (sc & 0x80) return false;

    // Código 0x2E = tecla 'C' (scancode set 1)
    return sc == 0x2E;
}

bool Keyboard::was_key_pressed(char key) {
    if (!keyboard_key_available())
        return false;

    uint8_t sc = keyboard_read_scancode();

    // Ignora tecla liberada (bit 7 = 1)
    if (sc & 0x80) return false;

    return sc == Console::asciiToScancode(key);
}
