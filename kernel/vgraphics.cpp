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

#define PI 3.14159265358979323846

static const float SIN_TABLE[360] = {
    0.000000f, 0.017452f, 0.034899f, 0.052336f, 0.069756f, 0.087156f, 0.104528f, 0.121869f, 0.139173f, 0.156434f,
    0.173648f, 0.190809f, 0.207912f, 0.224951f, 0.241922f, 0.258819f, 0.275637f, 0.292372f, 0.309017f, 0.325568f,
    0.342020f, 0.358368f, 0.374607f, 0.390731f, 0.406737f, 0.422618f, 0.438371f, 0.453990f, 0.469472f, 0.484810f,
    0.500000f, 0.515038f, 0.529919f, 0.544639f, 0.559193f, 0.573576f, 0.587785f, 0.601815f, 0.615661f, 0.629320f,
    0.642788f, 0.656059f, 0.669131f, 0.681998f, 0.694658f, 0.707107f, 0.719340f, 0.731354f, 0.743145f, 0.754710f,
    0.766044f, 0.777146f, 0.788011f, 0.798636f, 0.809017f, 0.819152f, 0.829038f, 0.838671f, 0.848048f, 0.857167f,
    0.866025f, 0.874620f, 0.882948f, 0.891007f, 0.898794f, 0.906308f, 0.913545f, 0.920505f, 0.927184f, 0.933580f,
    0.939693f, 0.945519f, 0.951057f, 0.956305f, 0.961262f, 0.965926f, 0.970296f, 0.974370f, 0.978148f, 0.981627f,
    0.984808f, 0.987688f, 0.990268f, 0.992546f, 0.994522f, 0.996195f, 0.997564f, 0.998630f, 0.999391f, 0.999848f,
    1.000000f, 0.999848f, 0.999391f, 0.998630f, 0.997564f, 0.996195f, 0.994522f, 0.992546f, 0.990268f, 0.987688f,
    0.984808f, 0.981627f, 0.978148f, 0.974370f, 0.970296f, 0.965926f, 0.961262f, 0.956305f, 0.951057f, 0.945519f,
    0.939693f, 0.933580f, 0.927184f, 0.920505f, 0.913545f, 0.906308f, 0.898794f, 0.891007f, 0.882948f, 0.874620f,
    0.866025f, 0.857167f, 0.848048f, 0.838671f, 0.829038f, 0.819152f, 0.809017f, 0.798636f, 0.788011f, 0.777146f,
    0.766044f, 0.754710f, 0.743145f, 0.731354f, 0.719340f, 0.707107f, 0.694658f, 0.681998f, 0.669131f, 0.656059f,
    0.642788f, 0.629320f, 0.615661f, 0.601815f, 0.587785f, 0.573576f, 0.559193f, 0.544639f, 0.529919f, 0.515038f,
    0.500000f, 0.484810f, 0.469472f, 0.453990f, 0.438371f, 0.422618f, 0.406737f, 0.390731f, 0.374607f, 0.358368f,
    0.342020f, 0.325568f, 0.309017f, 0.292372f, 0.275637f, 0.258819f, 0.241922f, 0.224951f, 0.207912f, 0.190809f,
    0.173648f, 0.156434f, 0.139173f, 0.121869f, 0.104528f, 0.087156f, 0.069756f, 0.052336f, 0.034899f, 0.017452f,
    0.000000f,-0.017452f,-0.034899f,-0.052336f,-0.069756f,-0.087156f,-0.104528f,-0.121869f,-0.139173f,-0.156434f,
    -0.173648f,-0.190809f,-0.207912f,-0.224951f,-0.241922f,-0.258819f,-0.275637f,-0.292372f,-0.309017f,-0.325568f,
    -0.342020f,-0.358368f,-0.374607f,-0.390731f,-0.406737f,-0.422618f,-0.438371f,-0.453990f,-0.469472f,-0.484810f,
    -0.500000f,-0.515038f,-0.529919f,-0.544639f,-0.559193f,-0.573576f,-0.587785f,-0.601815f,-0.615661f,-0.629320f,
    -0.642788f,-0.656059f,-0.669131f,-0.681998f,-0.694658f,-0.707107f,-0.719340f,-0.731354f,-0.743145f,-0.754710f,
    -0.766044f,-0.777146f,-0.788011f,-0.798636f,-0.809017f,-0.819152f,-0.829038f,-0.838671f,-0.848048f,-0.857167f,
    -0.866025f,-0.874620f,-0.882948f,-0.891007f,-0.898794f,-0.906308f,-0.913545f,-0.920505f,-0.927184f,-0.933580f,
    -0.939693f,-0.945519f,-0.951057f,-0.956305f,-0.961262f,-0.965926f,-0.970296f,-0.974370f,-0.978148f,-0.981627f,
    -0.984808f,-0.987688f,-0.990268f,-0.992546f,-0.994522f,-0.996195f,-0.997564f,-0.998630f,-0.999391f,-0.999848f,
    -1.000000f,-0.999848f,-0.999391f,-0.998630f,-0.997564f,-0.996195f,-0.994522f,-0.992546f,-0.990268f,-0.987688f,
    -0.984808f,-0.981627f,-0.978148f,-0.974370f,-0.970296f,-0.965926f,-0.961262f,-0.956305f,-0.951057f,-0.945519f,
    -0.939693f,-0.933580f,-0.927184f,-0.920505f,-0.913545f,-0.906308f,-0.898794f,-0.891007f,-0.882948f,-0.874620f,
    -0.866025f,-0.857167f,-0.848048f,-0.838671f,-0.829038f,-0.819152f,-0.809017f,-0.798636f,-0.788011f,-0.777146f,
    -0.766044f,-0.754710f,-0.743145f,-0.731354f,-0.719340f,-0.707107f,-0.694658f,-0.681998f,-0.669131f,-0.656059f,
    -0.642788f,-0.629320f,-0.615661f,-0.601815f,-0.587785f,-0.573576f,-0.559193f,-0.544639f,-0.529919f,-0.515038f,
    -0.500000f,-0.484810f,-0.469472f,-0.453990f,-0.438371f,-0.422618f,-0.406737f,-0.390731f,-0.374607f,-0.358368f,
    -0.342020f,-0.325568f,-0.309017f,-0.292372f,-0.275637f,-0.258819f,-0.241922f,-0.224951f,-0.207912f,-0.190809f,
    -0.173648f,-0.156434f,-0.139173f,-0.121869f,-0.104528f,-0.087156f,-0.069756f,-0.052336f,-0.034899f,-0.017452f
};

static inline float cos_lookup(int deg) {
    deg %= 360;
    if (deg < 0) deg += 360;
    return SIN_TABLE[(deg + 90) % 360];
}

static inline float sin_lookup(int deg) {
    deg %= 360;
    if (deg < 0) deg += 360;
    return SIN_TABLE[deg];
}

void VGraphics::draw_line_adv(int x0, int y0, int length, int deg, uint32_t color) {
    int x1 = x0 + (int)(cos_lookup(deg) * length);
    int y1 = y0 - (int)(sin_lookup(deg) * length); // restamos porque Y crece hacia abajo

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
