#include "vgraphics.h"

#include "font8x16/font8x16.h" // Fuente de 8x16

FBInfo VGraphics::fb;

void VGraphics::init(uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp, uintptr_t addr) {
	fb.width = width;
	fb.height = height;
	fb.pitch = pitch;
	fb.bpp = bpp;
    fb.address = reinterpret_cast<uint32_t*>(addr);

}

void VGraphics::putPixel(uint32_t x, uint32_t y, uint32_t color) {
    if (fb.bpp != 32) return;
    uint32_t offset = y * (fb.pitch / 4) + x;
    fb.address[offset] = color;
}

void VGraphics::drawChar(uint32_t x, uint32_t y, char c, Color fg) {
	if (c < 0 || c > 255) return;

	for (int row = 0; row < 16; row++) {
		uint8_t line = font8x16[(uint8_t)c][row];
		for (int col = 0; col < 8; col++) {
			if (line & (1 << (7 - col))) {
				putPixel(x + col, y + row, 0xffffff);
			}
		}
	}
}

void VGraphics::drawString(uint32_t x, uint32_t y, const char* str, Color fg) {
	while (*str) {
		drawChar(x, y, *str++, fg);
		x += 8;
	}
}

void VGraphics::clearScreen() {
    for (uint32_t y = 0; y < fb.height; y++) {
        for (uint32_t x = 0; x < fb.width; x++) {
            putPixel(x, y, 0x000000); // Color
        }
    }
}