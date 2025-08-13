#pragma once
#include <stdint.h>

namespace Debug {
    void Init();
    void Print(const char* s);
    void PrintChar(char c);
    void PrintHex(uint32_t val);
    void PrintDec(uint32_t val);
}
