#include "vesa.h"
#include "Console.h"
#include <stdint.h>

// Estructura mejorada para el framebuffer
struct Framebuffer {
    uint32_t* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    bool initialized;
    
    // Operador de acceso seguro
    uint32_t& pixel(uint32_t x, uint32_t y) {
        return address[y * (pitch / 4) + x];
    }
};

static Framebuffer framebuffer;

void init_graphics(struct multiboot_info_t* mbi) {
    if (!mbi || !(mbi->flags & (1 << 12))) {
        Console::println("Error: Informacion de framebuffer no disponible");
        return;
    }

    framebuffer.address = reinterpret_cast<uint32_t*>(mbi->framebuffer_addr);
    framebuffer.width = mbi->framebuffer_width;
    framebuffer.height = mbi->framebuffer_height;
    framebuffer.pitch = mbi->framebuffer_pitch;
    framebuffer.bpp = mbi->framebuffer_bpp;
    framebuffer.initialized = true;

    Console::println("Modo grafico inicializado: ", 
                    (long long unsigned)framebuffer.width, "x", (long long unsigned)framebuffer.height, 
                    " @ ", static_cast<void*>(framebuffer.address));
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!framebuffer.initialized || x >= framebuffer.width || y >= framebuffer.height) {
        return;
    }
    framebuffer.pixel(x, y) = color;  // Uso del método seguro
}

void gclear_screen(uint32_t color) {
    if (!framebuffer.initialized) return;
    
    for (uint32_t y = 0; y < framebuffer.height; y++) {
        for (uint32_t x = 0; x < framebuffer.width; x++) {
            framebuffer.pixel(x, y) = color;
        }
    }
}