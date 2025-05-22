#include "Console.h"
#include "io.h"
#define VGA_ADDRESS 0xB8000

uint16_t  Console::cursorPos    = 0;
uint8_t   Console::color        = 0x07;
uint16_t* Console::vgaBuffer    = (uint16_t*)VGA_ADDRESS;
char      Console::lineBuffer[] = {0};

void Console::clearScreen() {
    uint16_t blank = (color << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        vgaBuffer[i] = blank;

    cursorPos = 0;
    updateCursor();
}

void Console::setColor(uint8_t newColor) {
    color = newColor;
}

void Console::enable_cursor(uint8_t start, uint8_t end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void Console::set_cursor(uint16_t pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
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

    set_cursor(cursorPos);
}

void Console::write(const char* str) {
    if (!str) return;
    while (*str) putChar(*str++);
}

void Console::write(const char* str, uint8_t fg, uint8_t bg) {
    // Guardamos color actual
    uint8_t oldColor = color;

    // Seteamos nuevo color
    color = (bg << 4) | (fg & 0x0F);

    // Escribimos
    write(str);

    // Restauramos color anterior
    color = oldColor;
}
void Console::write(const char* str, uint8_t fg) {
    write(str, fg, (color >> 4)); // usa fondo actual
}


void Console::write(char c) {
    putChar(c);
}

void Console::write(int value) {
    char buffer[12];
    itoa(value, buffer, 10);
    write(buffer);
}

void Console::write(uint8_t value) {
    write((int)value);
}

void Console::write(bool value) {
    write(value ? "true" : "false");
}

void Console::println() {
    putChar('\n');
}

void Console::scroll() {
    for (int y = 1; y < VGA_HEIGHT; ++y)
        for (int x = 0; x < VGA_WIDTH; ++x)
            vgaBuffer[(y - 1) * VGA_WIDTH + x] = vgaBuffer[y * VGA_WIDTH + x];

    uint16_t blank = (color << 8) | ' ';
    for (int x = 0; x < VGA_WIDTH; ++x)
        vgaBuffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;

    cursorPos -= VGA_WIDTH;
}

void Console::updateCursor() {
    uint16_t pos = cursorPos;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// --- Teclado ---

extern "C" int getKey() {
    uint8_t status, sc;

    while (true) {
        do {
            asm volatile("inb $0x64, %0" : "=a"(status));
        } while ((status & 1) == 0);

        asm volatile("inb $0x60, %0" : "=a"(sc));
        if (sc & 0x80) continue;
        return sc;
    }
}

#define KEY_LEFT       0x4B
#define KEY_RIGHT      0x4D
#define KEY_UP         0x48
#define KEY_DOWN       0x50
#define KEY_BACKSPACE  0x0E
#define KEY_ENTER      0x1C

char scancodeToAscii(uint8_t sc) {
    static const char table[] = {
        0, 27, '1','2','3','4','5','6','7','8',
        '9','0','-','=','\b','\t','q','w','e','r',
        't','y','u','i','o','p','[',']','\n',0,
        'a','s','d','f','g','h','j','k','l',';',
        '\'', '`', 0, '\\','z','x','c','v','b','n',
        'm',',','.','/', 0, '*', 0, ' '
    };
    if (sc < sizeof(table)) return table[sc];
    return 0;
}

// --- Línea editable ---

char* Console::readLine(char* buffer, int maxLength) {
    int length = 0;
    int cursor = 0;
    int startPos = cursorPos;

    while (true) {
        int sc = getKey();

        if (sc == KEY_ENTER) {
            putChar('\n');
            break;
        }

        else if (sc == KEY_BACKSPACE) {
            if (cursor > 0) {
                for (int i = cursor - 1; i < length - 1; ++i)
                    buffer[i] = buffer[i + 1];
                length--;
                cursor--;

                // Borrar carácter visualmente
                for (int i = cursor; i < length; ++i)
                    vgaBuffer[startPos + i] = (color << 8) | buffer[i];
                vgaBuffer[startPos + length] = (color << 8) | ' ';

                cursorPos = startPos + cursor;
                updateCursor();
            }
        }

        else if (sc == KEY_LEFT) {
            if (cursor > 0) {
                cursor--;
                cursorPos--;
                updateCursor();
            }
        }

        else if (sc == KEY_RIGHT) {
            if (cursor < length) {
                cursor++;
                cursorPos++;
                updateCursor();
            }
        }

        else {
            char c = scancodeToAscii(sc);
            if (c && length < maxLength - 1) {
                for (int i = length; i > cursor; --i)
                    buffer[i] = buffer[i - 1];
                buffer[cursor] = c;
                length++;
                cursor++;

                // Redibujar línea
                for (int i = cursor - 1; i < length; ++i)
                    vgaBuffer[startPos + i] = (color << 8) | buffer[i];

                cursorPos = startPos + cursor;
                updateCursor();
            }
        }
    }

    buffer[length] = '\0';
    return buffer;
}

void Console::itoa(int value, char* str, int base) {
    char* ptr = str;
    bool isNegative = false;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        *ptr++ = (rem < 10) ? rem + '0' : rem - 10 + 'A';
        value /= base;
    }

    if (isNegative)
        *ptr++ = '-';

    *ptr = '\0';

    // Invierte el string
    for (char* start = str, *end = ptr - 1; start < end; ++start, --end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
    }
}

