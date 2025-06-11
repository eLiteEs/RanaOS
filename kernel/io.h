#pragma once
#include <stdint.h>

extern "C" {
    char* readLineASM();
    int   getKey();           // devuelve ASCII
    void  putc(char c);       // imprime un carácter

    uint8_t inb(uint16_t port);
    void    outb(uint16_t port, uint8_t value);

    uint16_t inw(uint16_t port);
    void outw(uint16_t port, uint16_t val);

    void insw(uint16_t port, void* addr, int count); 
}