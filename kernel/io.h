#pragma once
#include <stdint.h>

extern "C" {
    char* readLineASM();
    int   getKey();           // devuelve ASCII
    void  putc(char c);       // imprime un carácter
}

