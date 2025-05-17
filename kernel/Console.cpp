#include "Console.h"
#define VGA_ADDRESS 0xB8000

uint16_t  Console::cursorPos    = 0;
uint8_t   Console::color        = 0x07;
uint16_t* Console::vgaBuffer    = (uint16_t*)VGA_ADDRESS;
char      Console::lineBuffer[] = {0};

extern "C" uint8_t getKey();  // Definido en ensamblador

void Console::clearScreen() {
    uint16_t blank = (color << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH*VGA_HEIGHT; ++i) {
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
    if (cursorPos >= VGA_WIDTH*VGA_HEIGHT) scroll();
    updateCursor();
}

void Console::write(const char* str) {
    if (!str) return;
    while (*str) putChar(*str++);
}

void Console::scroll() {
    for (int y = 1; y < VGA_HEIGHT; ++y)
        for (int x = 0; x < VGA_WIDTH; ++x)
            vgaBuffer[(y-1)*VGA_WIDTH + x] = vgaBuffer[y*VGA_WIDTH + x];

    uint16_t blank = (color << 8) | ' ';
    for (int x = 0; x < VGA_WIDTH; ++x)
        vgaBuffer[(VGA_HEIGHT-1)*VGA_WIDTH + x] = blank;

    cursorPos -= VGA_WIDTH;
}

void Console::updateCursor() {
    uint16_t pos = cursorPos;
    asm volatile("outb %0, %%dx" : : "a"((uint8_t)(pos & 0xFF)), "d"(0x3D5));
    asm volatile("outb %0, %%dx" : : "a"((uint8_t)((pos>>8)&0xFF)), "d"(0x3D4));
}

char* Console::readLine(char* buffer, int maxLength) {
    int length = 0;

    while (true) {
        char c = getKey();

        if (c == '\r' || c == '\n') {
            putChar('\n');
            break;
        }
        else if (c == 8) {  // Backspace
            if (length > 0) {
                length--;
                putChar('\b');
                putChar(' ');
                putChar('\b');
            }
        }
        else if (c >= 32 && c <= 126 && length < maxLength - 1) {
            buffer[length++] = c;
            putChar(c);
        }
        // otros códigos (flechas) podrías añadirlos luego...
    }

    buffer[length] = '\0';
    return buffer;
}

void Console::updateLineBufferDisplay(int cursor, int length) {
    for (int i = 0; i < length; i++) {
        vgaBuffer[i] = (color << 8) | lineBuffer[i];
    }
    for (int i = length; i < VGA_WIDTH; i++) {
        vgaBuffer[i] = (color << 8) | ' ';
    }
    cursorPos = cursor;
    updateCursor();
}
