#include "Debug.h"

extern "C" {

void debug_print(const char* m) {
    Debug::Print(m);
}

void debug_print_dec(uint32_t n) {
    Debug::PrintDec(n);
}

void debug_print_hex(uint32_t h) {
    Debug::PrintHex(h);
}

}
