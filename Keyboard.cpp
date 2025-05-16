// keyboard.cpp
#include "Keyboard.h"
#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define PIC1_COMMAND 0x20
#define PIC_EOI 0x20

static volatile int keyBuffer[256];
static volatile int head = 0;
static volatile int tail = 0;

extern "C" void keyboard_handler();

char scancodeToAscii(uint8_t sc); 

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void keyboard_install() {
    // Aquí instalas la interrupción IRQ1 y el handler keyboard_handler
    // No está implementado en este snippet
}

int getKey() {
    if (head == tail) return 0;
    int code = keyBuffer[tail];
    tail = (tail + 1) % 256;
    return code;
}

extern "C" void keyboard_handler() {
    static bool extended = false;

    uint8_t sc = inb(KEYBOARD_DATA_PORT);

    if (sc == 0xE0) {
        extended = true;
        return;
    }

    int key = 0;
    if (extended) {
        // Mapear teclas extendidas (flechas)
        switch(sc) {
            case 0x4B: key = -1; break; // flecha izquierda
            case 0x4D: key = -2; break; // flecha derecha
            case 0x48: key = -3; break; // flecha arriba
            case 0x50: key = -4; break; // flecha abajo
            default: key = 0;
        }
        extended = false;
    } else {
        // Mapear scancode normal a ASCII, solo algunas letras para ejemplo
        key = scancodeToAscii(sc);
    }

    if (key) {
        keyBuffer[head] = key;
        head = (head + 1) % 256;
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

static const char scancodeMap[128] = {
    /*0x00*/ 0,    /*0x01*/ 0,    /*0x02*/ '1',  /*0x03*/ '2',
    /*0x04*/ '3',  /*0x05*/ '4',  /*0x06*/ '5',  /*0x07*/ '6',
    /*0x08*/ '7',  /*0x09*/ '8',  /*0x0A*/ '9',  /*0x0B*/ '0',
    /*0x0C*/ '-',  /*0x0D*/ '=',  /*0x0E*/ '\b', /*Backspace*/
    /*0x0F*/ '\t', /*Tab*/
    /*0x10*/ 'q',  /*0x11*/ 'w',  /*0x12*/ 'e',  /*0x13*/ 'r',
    /*0x14*/ 't',  /*0x15*/ 'y',  /*0x16*/ 'u',  /*0x17*/ 'i',
    /*0x18*/ 'o',  /*0x19*/ 'p',  /*0x1A*/ '[',  /*0x1B*/ ']',
    /*0x1C*/ '\n', /*Enter*/
    /*0x1D*/ 0,    /*Ctrl*/      /*0x1E*/ 'a',  /*0x1F*/ 's',
    /*0x20*/ 'd',  /*0x21*/ 'f',  /*0x22*/ 'g',  /*0x23*/ 'h',
    /*0x24*/ 'j',  /*0x25*/ 'k',  /*0x26*/ 'l',  /*0x27*/ ';',
    /*0x28*/ '\'', /*0x29*/ '`',  /*0x2A*/ 0,    /*Shift*/
    /*0x2B*/ '\\', /*0x2C*/ 'z',  /*0x2D*/ 'x',  /*0x2E*/ 'c',
    /*0x2F*/ 'v',  /*0x30*/ 'b',  /*0x31*/ 'n',  /*0x32*/ 'm',
    /*0x33*/ ',',  /*0x34*/ '.',  /*0x35*/ '/',  /*0x36*/ 0,    
    /*R-Shift*/   /*0x37*/ '*',  /*KP **/     /*0x38*/ 0,    /*Alt*/
    /*0x39*/ ' ',  /*Space*/
    /*0x3A*/ 0,    /*CapsLock*/
    /*0x3B*/ 0,    /*F1*/       /*0x3C*/ 0,    /*F2*/
    /*0x3D*/ 0,    /*F3*/       /*0x3E*/ 0,    /*F4*/
    /*0x3F*/ 0,    /*F5*/       /*0x40*/ 0,    /*F6*/
    /*0x41*/ 0,    /*F7*/       /*0x42*/ 0,    /*F8*/
    /*0x43*/ 0,    /*F9*/       /*0x44*/ 0,    /*F10*/
    /*0x45*/ 0,    /*NumLock*/ /*0x46*/ 0,    /*ScrollLock*/
    /*0x47*/ 0,    /*KP 7*/    /*0x48*/ 0,    /*KP 8*/
    /*0x49*/ 0,    /*KP 9*/    /*0x4A*/ 0,    /*KP -*/
    /*0x4B*/ 0,    /*KP 4*/    /*0x4C*/ 0,    /*KP 5*/
    /*0x4D*/ 0,    /*KP 6*/    /*0x4E*/ 0,    /*KP +*/
    /*0x4F*/ 0,    /*KP 1*/    /*0x50*/ 0,    /*KP 2*/
    /*0x51*/ 0,    /*KP 3*/    /*0x52*/ 0,    /*KP 0*/
    /*0x53*/ 0,    /*KP .*/    /*0x54*/ 0,    /*F11*/
    /*0x55*/ 0,    /*F12*/     /*0x56*/ 0,    /*? OEM*/
    /*0x57*/ 0,    /*F13…*/    /*0x58*/ 0
};

// Tabla con Shift pulsado:
static const char scancodeMapShift[128] = {
    /*0x00*/ 0,    /*0x01*/ 0,    /*0x02*/ '!',  /*0x03*/ '@',
    /*0x04*/ '#',  /*0x05*/ '$',  /*0x06*/ '%',  /*0x07*/ '^',
    /*0x08*/ '&',  /*0x09*/ '*',  /*0x0A*/ '(',  /*0x0B*/ ')',
    /*0x0C*/ '_',  /*0x0D*/ '+',  /*0x0E*/ '\b', /*Backspace*/
    /*0x0F*/ '\t', /*Tab*/
    /*0x10*/ 'Q',  /*0x11*/ 'W',  /*0x12*/ 'E',  /*0x13*/ 'R',
    /*0x14*/ 'T',  /*0x15*/ 'Y',  /*0x16*/ 'U',  /*0x17*/ 'I',
    /*0x18*/ 'O',  /*0x19*/ 'P',  /*0x1A*/ '{',  /*0x1B*/ '}',
    /*0x1C*/ '\n', /*Enter*/
    /*0x1D*/ 0,    /*Ctrl*/      /*0x1E*/ 'A',  /*0x1F*/ 'S',
    /*0x20*/ 'D',  /*0x21*/ 'F',  /*0x22*/ 'G',  /*0x23*/ 'H',
    /*0x24*/ 'J',  /*0x25*/ 'K',  /*0x26*/ 'L',  /*0x27*/ ':',
    /*0x28*/ '"',  /*0x29*/ '~',  /*0x2A*/ 0,    /*Shift*/
    /*0x2B*/ '|',  /*0x2C*/ 'Z',  /*0x2D*/ 'X',  /*0x2E*/ 'C',
    /*0x2F*/ 'V',  /*0x30*/ 'B',  /*0x31*/ 'N',  /*0x32*/ 'M',
    /*0x33*/ '<',  /*0x34*/ '>',  /*0x35*/ '?',  /*0x36*/ 0,
    /*R-Shift*/   /*0x37*/ '*',  /*KP **/     /*0x38*/ 0,    /*Alt*/
    /*0x39*/ ' ',  /*Space*/
    /*0x3A*/ 0,    /*CapsLock*/
    /*0x3B*/ 0,    /*F1*/       /*0x3C*/ 0,    /*F2*/
    /*0x3D*/ 0,    /*F3*/       /*0x3E*/ 0,    /*F4*/
    /*0x3F*/ 0,    /*F5*/       /*0x40*/ 0,    /*F6*/
    /*0x41*/ 0,    /*F7*/       /*0x42*/ 0,    /*F8*/
    /*0x43*/ 0,    /*F9*/       /*0x44*/ 0,    /*F10*/
    /*0x45*/ 0,    /*NumLock*/ /*0x46*/ 0,    /*ScrollLock*/
    /*0x47*/ 0,    /*KP 7*/    /*0x48*/ 0,    /*KP 8*/
    /*0x49*/ 0,    /*KP 9*/    /*0x4A*/ 0,    /*KP -*/
    /*0x4B*/ 0,    /*KP 4*/    /*0x4C*/ 0,    /*KP 5*/
    /*0x4D*/ 0,    /*KP 6*/    /*0x4E*/ 0,    /*KP +*/
    /*0x4F*/ 0,    /*KP 1*/    /*0x50*/ 0,    /*KP 2*/
    /*0x51*/ 0,    /*KP 3*/    /*0x52*/ 0,    /*KP 0*/
    /*0x53*/ 0,    /*KP .*/    /*0x54*/ 0,    /*F11*/
    /*0x55*/ 0,    /*F12*/     /*0x56*/ 0,    /*? OEM*/
    /*0x57*/ 0,    /*F13…*/    /*0x58*/ 0
};

// Estado global de Shift
static bool shiftDown = false;

// Función de traducción completa
char scancodeToAscii(uint8_t sc) {
    // Teclas Make: 0x02..0x58, Break codes suman 0x80
    bool release = sc & 0x80;
    uint8_t code = sc & 0x7F;

    // Detectar Shift
    if (!release) {
        if (code == 0x2A || code == 0x36) { shiftDown = true;  return 0; }
    } else {
        if (code == 0x2A || code == 0x36) { shiftDown = false; return 0; }
    }

    if (release) {
        // tecla soltada, no devolvemos carácter
        return 0;
    }

    // Devuelve según el estado de Shift
    if (shiftDown) {
        return scancodeMapShift[code];
    } else {
        return scancodeMap[code];
    }
}
