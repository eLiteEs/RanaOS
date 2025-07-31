#pragma once

#include <stdint.h>

struct Color {
    uint8_t r, g, b, a;
};

struct FramebufferInfo {
    void* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint8_t memory_model;
};

class VGraphics {
public:
    static void initialize(FramebufferInfo* fbInfo);
    static void putPixel(uint32_t x, uint32_t y, Color color);
    static void drawChar(uint32_t x, uint32_t y, char c, Color fg, Color bg);
    static void drawString(uint32_t x, uint32_t y, const char* str, Color fg, Color bg);
    static int getPitch();
    static int getWidth();
    static int getHeight();
    static void* getFramebuffer() { return fbInfo.address; }
private:
    static FramebufferInfo fbInfo;
    
};