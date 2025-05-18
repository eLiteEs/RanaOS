#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

class Console {
public:
    static void clearScreen();
    static void setColor(uint8_t newColor);
    static void putChar(char c);
    static void write(const char* str);
    static void write(char c);
    static void write(int value);
    static void write(uint8_t value);
    static void write(bool value);

    static void println(); // fin de línea sin argumentos
    template<typename T, typename... Args>
    static void println(T first, Args... rest);

    static char* readLine();
    static void updateLineBufferDisplay(int cursor, int length);
    static char* readLine(char* buffer, int maxLength);

private:
    static const uint16_t VGA_WIDTH  = 80;
    static const uint16_t VGA_HEIGHT = 25;
    static const int MAXLEN = 256;

    static uint16_t  cursorPos;
    static uint8_t   color;
    static uint16_t* vgaBuffer;
    static char      lineBuffer[MAXLEN];

    static void updateCursor();
    static void scroll();

    static void itoa(int value, char* str, int base); // 👈 añade esta
};

// Implementación de la plantilla en el header
template<typename T, typename... Args>
void Console::println(T first, Args... rest) {
    write(first);
    println(rest...);
}

#endif // CONSOLE_H

