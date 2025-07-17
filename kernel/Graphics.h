// graphics.h
#pragma once
#include <stdint.h>

class Graphics {
public:
    static void init();
    static void put_pixel(uint16_t x, uint16_t y, uint8_t color);
    static void clear_screen(uint8_t color);
    static void switch_to_text_mode();
    
    static constexpr uint16_t WIDTH = 320;
    static constexpr uint16_t HEIGHT = 200;

    static void DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
private:
    static void write_registers(uint8_t* regs);
};