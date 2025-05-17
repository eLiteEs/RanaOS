#include "Console.h"

static char linebuf[256];

extern "C" void kmain() {
    Console::clearScreen();
    Console::write("RanaOS alpha 2\n\n");

    while (1) {
        Console::write("$- ");
        char* s = Console::readLine(linebuf, sizeof(linebuf));
        Console::write("Has escrito: ");
        Console::write(s);
        Console::putChar('\n');
    }
}

