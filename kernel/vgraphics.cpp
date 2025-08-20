#include "vgraphics.h"

#include "font8x16/font8x16.h" // Fuente de 8x16
#include "string.h" // Para memcpy

float absf(float x) { return (x < 0) ? -x : x; }
int absi(int x) { return (x < 0) ? -x : x; }
float sinf(float x);
float cosf(float x);
float tanf(float x);

// Implementación de funciones trigonométricas básicas
float sinf(float x) {
    // Aproximación con serie de Taylor
    float result = x;
    float term = x;
    float x2 = x * x;
    for (int n = 1; n < 5; n++) {
        term *= -x2 / ((2*n) * (2*n+1));
        result += term;
    }
    return result;
}

float cosf(float x) {
    // Aproximación con serie de Taylor
    float result = 1.0f;
    float term = 1.0f;
    float x2 = x * x;
    for (int n = 1; n < 5; n++) {
        term *= -x2 / ((2*n-1) * (2*n));
        result += term;
    }
    return result;
}

float tanf(float x) {
    float c = cosf(x);
    return (c != 0) ? sinf(x)/c : 0; // Evitar división por cero
}

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

void VGraphics::draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = absi(x1 - x0);
    int dy = -absi(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        VGraphics::putPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}
void VGraphics::draw_line_adv(int x0, int y0, int width, int deg, uint32_t color) {	
    double rad = deg * 3.14159265358979323846 / 180.0;

    int x1 = x0 + (int)(cosf(rad) * width);
    int y1 = y0 + (int)((sinf(rad) - sinf(rad) - sinf(rad)) * width);

    VGraphics::draw_line(x0, y0, x1, y1, color);
}
void VGraphics::draw_circle(int cx, int cy, int r, uint32_t color) {
    int x = r;
    int y = 0;
    int decision = 1 - x;

    while (x >= y) {
        VGraphics::putPixel(cx + x, cy + y, color);
        VGraphics::putPixel(cx - x, cy + y, color);
        VGraphics::putPixel(cx + x, cy - y, color);
        VGraphics::putPixel(cx - x, cy - y, color);
        VGraphics::putPixel(cx + y, cy + x, color);
        VGraphics::putPixel(cx - y, cy + x, color);
        VGraphics::putPixel(cx + y, cy - x, color);
        VGraphics::putPixel(cx - y, cy - x, color);

        y++;
        if (decision <= 0) {
            decision += 2 * y + 1;
        } else {
            x--;
            decision += 2 * (y - x) + 1;
        }
    }
}

void VGraphics::fill_circle(int cx, int cy, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
	      VGraphics::putPixel(cx + x, cy + y, color);
            }
        }
    }
}
