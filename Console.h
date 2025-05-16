#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

class Console {
public:
    static void clearScreen();
    static void setColor(uint8_t newColor);
    static void putChar(char c);
    static void write(const char* str);

    // Ahora readLine devuelve un buffer interno estático
    // (hasta MAXLEN-1 caracteres + '\0').
    static char* readLine();

private:
    static const uint16_t VGA_WIDTH  = 80;
    static const uint16_t VGA_HEIGHT = 25;
    static const int MAXLEN = 256;

    static uint16_t cursorPos;
    static uint8_t  color;
    static uint16_t* vgaBuffer;

    // Buffer interno para readLine
    static char lineBuffer[MAXLEN];

    static void updateCursor();
    static void scroll();
};

#endif // CONSOLE_H

