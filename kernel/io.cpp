#include "Console.h"
#include "io.h"

extern "C" void putc(char c) {
    Console::putChar(c);
}

