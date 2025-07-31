#pragma once
#include <stdint.h>

namespace Mouse {
    void initialize();
    int16_t get_x();
    int16_t get_y();
    uint8_t get_buttons();
    void set_boundaries(uint16_t width, uint16_t height);
}