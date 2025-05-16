#include "Console.h"
#include "Keyboard.h"    // getKey()
#include <stddef.h>      // para NULL, opcional

#define VGA_ADDRESS 0xB8000

// Definición de miembros estáticos
uint16_t  Console::cursorPos    = 0;
uint8_t   Console::color        = 0x07;
uint16_t* Console::vgaBuffer    = (uint16_t*)VGA_ADDRESS;
char      Console::lineBuffer[] = {0};

void Console::clearScreen() {
    uint16_t blank = (color << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vgaBuffer[i] = blank;
    }
    cursorPos = 0;
    updateCursor();
}

void Console::setColor(uint8_t newColor) {
    color = newColor;
}

void Console::putChar(char c) {
    if (c == '\n') {
        cursorPos += VGA_WIDTH - (cursorPos % VGA_WIDTH);
    } else if (c == '\r') {
        cursorPos -= cursorPos % VGA_WIDTH;
    } else {
        vgaBuffer[cursorPos++] = (color << 8) | c;
    }
    if (cursorPos >= VGA_WIDTH * VGA_HEIGHT) {
        scroll();
    }
    updateCursor();
}

void Console::write(const char* str) {
    if (!str) return;
    while (*str) {
        putChar(*str++);
    }
}

void Console::scroll() {
    for (int y = 1; y < VGA_HEIGHT; ++y) {
        for (int x = 0; x < VGA_WIDTH; ++x) {
            vgaBuffer[(y-1)*VGA_WIDTH + x] = vgaBuffer[y*VGA_WIDTH + x];
        }
    }
    uint16_t blank = (color << 8) | ' ';
    for (int x = 0; x < VGA_WIDTH; ++x) {
        vgaBuffer[(VGA_HEIGHT-1)*VGA_WIDTH + x] = blank;
    }
    cursorPos -= VGA_WIDTH;
}

void Console::updateCursor() {
    uint16_t pos = cursorPos;
    asm volatile ("outb %0, $0x3D4" : : "a"(0x0F));
    asm volatile ("outb %0, $0x3D5" : : "a"((uint8_t)(pos & 0xFF)));
    asm volatile ("outb %0, $0x3D4" : : "a"(0x0E));
    asm volatile ("outb %0, $0x3D5" : : "a"((uint8_t)((pos>>8)&0xFF)));
}

char* Console::readLine() {
    int idx = 0;
    // limpia buffer
    for (int i = 0; i < MAXLEN; ++i) lineBuffer[i] = 0;

    while (true) {
        int c = 0;
        while (!(c = getKey())) { /* espera activa */ }

        if (c == '\n' || c == '\r') {
            putChar('\n');
            break;
        }
        else if (c == '\b') {
            if (idx > 0) {
                --idx;
                putChar('\b');
                putChar(' ');
                putChar('\b');
            }
        }
        else if (c < 0) {
            // Flechas u otras, ignóralas o maneja aquí
        }
        else {
            if (idx < MAXLEN - 1) {
                lineBuffer[idx++] = (char)c;
                putChar((char)c);
            }
        }
    }

    lineBuffer[idx] = '\0';
    return lineBuffer;
}

