#include "vgraphics.h"

#include "font8x16/font8x16.h" // Fuente de 8x16
#include "string.h" // Para memcpy

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

void VGraphics::drawChar(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bg) {
    if (x >= fb.width || y >= fb.height) return; // Fuera de pantalla
	if (c < 0 || c > 255) return;

    for(int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++)
        {
            putPixel(x + col, y + row, bg); // Fondo
        }
    }   

	for (int row = 0; row < 16; row++) {
		uint8_t line = font8x16[(uint8_t)c][row];
		for (int col = 0; col < 8; col++) {
			if (line & (1 << (7 - col))) {
				putPixel(x + col, y + row, color);
			}
		}
	}
}

void VGraphics::drawString(uint32_t x, uint32_t y, const char* str, uint32_t fg, uint32_t bg) {
	while (*str) {
		drawChar(x, y, *str++, fg, bg);
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

void VGraphics::fillRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < width; j++) {
			putPixel(x + j, y + i, color);
		}
	}
}

void VGraphics::scroll() {
    if (fb.bpp != 32) return;

    uint8_t* fbRaw = (uint8_t*)fb.address;
    uint32_t lineHeight = 16;

    for (uint32_t y = 0; y < fb.height - lineHeight; ++y) {
        memcpy(
            fbRaw + y * fb.pitch,
            fbRaw + (y + lineHeight) * fb.pitch,
            fb.pitch
        );
    }

    for (uint32_t y = fb.height - lineHeight; y < fb.height; ++y) {
        uint32_t* row = (uint32_t*)(fbRaw + y * fb.pitch);
        for (uint32_t x = 0; x < fb.width; ++x) {
            row[x] = 0x00000000;
        }
    }
}
