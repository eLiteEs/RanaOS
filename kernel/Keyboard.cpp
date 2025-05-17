#include "Keyboard.h"
#include "Console.h"
#include "pic.h"
#include "idt.h"
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC_EOI       0x20

// Buffer circular de teclas
static volatile int head = 0, tail = 0;
static volatile int keyBuffer[256];
static bool shiftDown = false;

// Traducción PS/2 Set 1 sin Shift / con Shift
static const char scancodeMap[128] = {
  /*0x00*/0, 27,'1','2','3','4','5','6',
  '7','8','9','0','-','=', '\b','\t',
  'q','w','e','r','t','y','u','i',
  'o','p','[',']','\n', 0,'a','s',
  'd','f','g','h','j','k','l',';',
  '\'','`',0,'\\','z','x','c','v',
  'b','n','m',',','.','/',0,'*',
  0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static const char scancodeMapShift[128] = {
  0, 27,'!','@','#','$','%','^',
  '&','*','(',')','_','+','\b','\t',
  'Q','W','E','R','T','Y','U','I',
  'O','P','{','}','\n',0,'A','S',
  'D','F','G','H','J','K','L',':',
  '"','~',0,'|','Z','X','C','V',
  'B','N','M','<','>','?',0,'*',
  0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
/*
char scancodeToAscii(uint8_t sc) {
    bool release = sc & 0x80;
    uint8_t code = sc & 0x7F;

    if (code == 0x2A || code == 0x36) {
        shiftDown = !release;
        return 0;
    }
    if (release) return 0;

    return shiftDown ? scancodeMapShift[code] : scancodeMap[code];
}*/

extern "C" void keyboard_handler() {
    uint8_t sc = inb(0x60);
    char c = scancodeToAscii(sc);
    if (c) {
        keyBuffer[head] = c;
        head = (head + 1) % 256;
        Console::putChar(c);  // Para ver que llega la tecla en pantalla
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

//int getKey() {
//    int c;
//    do {
//        c = 0;
//        if (head != tail) {
//            c = keyBuffer[tail];
//            tail = (tail + 1) % 256;
//            Console::putChar(c);  // Debug: mostrar tecla extraída
//        }
//    } while (c == 0);
//    return c;
//}


void keyboard_install() {
    remap_pic();
    set_idt_gate(0x21, (uint32_t)keyboard_handler, 0x08, 0x8E);
    enable_irq(1);
    asm volatile("sti");
}


// scancodeToAscii y códigos especiales
#define KEY_LEFT  0x4B
#define KEY_RIGHT 0x4D
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_BACKSPACE 0x0E
#define KEY_ENTER 0x1C

char scancodeToAscii(uint8_t sc) {
    static const char table[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', // 0x00 - 0x09
        '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', // 0x0A - 0x13
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, // 0x14 - 0x1D
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x1E - 0x27
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', // 0x28 - 0x31
        'm', ',', '.', '/', 0, '*', 0, ' ' // ...hasta 0x39
    };

    if (sc < sizeof(table)) return table[sc];
    return 0;
}

extern "C" int getKey() {
    uint8_t status, sc;

    while (true) {
        // Espera a que el controlador de teclado tenga dato
        do {
            asm volatile("inb $0x64, %0" : "=a"(status));
        } while ((status & 1) == 0);

        // Lee el scancode
        asm volatile("inb $0x60, %0" : "=a"(sc));

        // Ignora el "key up"
        if (sc & 0x80) continue;

        return sc;
    }
}
