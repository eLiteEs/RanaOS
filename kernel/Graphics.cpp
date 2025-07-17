// graphics.cpp
#include "Graphics.h"
#include "io.h"

static uint8_t* const VGA = (uint8_t*)0xA0000;

// Registros para el modo 13h (320x200, 256 colores)
static uint8_t g_320x200x256[] = {
    /* MISC */
    0x63,
    /* SEQ */
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

void Graphics::write_registers(uint8_t* regs) {
    // MISC
    outb(0x3C2, *regs++);
    
    // SEQ
    for(uint8_t i = 0; i < 5; i++) {
        outb(0x3C4, i);
        outb(0x3C5, *regs++);
    }
    
    // CRTC
    for(uint8_t i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, *regs++);
    }
    
    // GC
    for(uint8_t i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, *regs++);
    }
    
    // AC
    for(uint8_t i = 0; i < 21; i++) {
        inb(0x3DA);
        outb(0x3C0, i);
        outb(0x3C0, *regs++);
    }
    
    // Habilitar modo gráfico
    inb(0x3DA);
    outb(0x3C0, 0x20);
}

void Graphics::init() {
    write_registers(g_320x200x256);
    
    // Configurar plano de memoria
    outb(0x3C4, 0x02);
    outb(0x3C5, 0x0F);
    
    // Limpiar pantalla
    clear_screen(0);
}

void Graphics::put_pixel(uint16_t x, uint16_t y, uint8_t color) {
    if(x >= WIDTH || y >= HEIGHT) return;
    VGA[y * WIDTH + x] = color;
}

void Graphics::clear_screen(uint8_t color) {
    for(uint32_t i = 0; i < WIDTH * HEIGHT; i++) {
        VGA[i] = color;
    }
}

void Graphics::switch_to_text_mode() {
    // 1. Esperar retrace vertical para evitar glitches
    while((inb(0x3DA) & 0x08));
    while(!((inb(0x3DA) & 0x08)));
    
    // 2. Secuencia de reset VGA segura
    outb(0x3C4, 0x01); outb(0x3C5, 0x00); // Deshabilitar protección
    outb(0x3C4, 0x00); outb(0x3C5, 0x03); // Reset secuenciador
    outb(0x3C4, 0x01); outb(0x3C5, 0x01); // Clocking mode
    
    // 3. Configuración mínima de modo texto
    uint8_t crtc[] = {0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, 
                      0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00};
    for(uint8_t i = 0; i < 16; i++) {
        outb(0x3D4, i); outb(0x3D5, crtc[i]);
    }
    
    // 4. Limpieza de pantalla con color seguro
    uint16_t* vga = (uint16_t*)0xB8000;
    for(int i = 0; i < 80*25; i++) {
        vga[i] = 0x0720; // Gris sobre negro
    }
    
    // 5. Forzar refresco en VirtualBox
    for(int i = 0; i < 10; i++) {
        inb(0x3DA); outb(0x3C0, 0x20);
    }
}

void Graphics::DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    for(int i = y; i < y + height; i++) {
        for(int j = x; j < x + width; j++) {
            put_pixel(j, i, color);
        }
    }
}