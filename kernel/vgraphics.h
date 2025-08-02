#pragma once
#include <stdint.h>

struct Color {
	uint8_t r, g, b, a;
	Color(uint8_t r=0, uint8_t g=0, uint8_t b=0, uint8_t a=0)
		: r(r), g(g), b(b), a(a) {}
};

struct FBInfo {
	uint32_t* address;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
};

class VGraphics {
public:
	static void init(uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp, uintptr_t addr);
	static void putPixel(uint32_t x, uint32_t y, uint32_t color);
    static void drawChar(uint32_t x, uint32_t y, char c, Color fg);
    static void drawString(uint32_t x, uint32_t y, const char* str, Color fg);
    static void clearScreen();

private:
	static FBInfo fb;
};
