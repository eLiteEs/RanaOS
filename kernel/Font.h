// font.h
#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Declaración exacta como en el repositorio
extern char font8x8_basic[128][8];

// Función para establecer fuente
void gfx_set_font(const char (*font)[8]);  // Puntero a arrays de 8 chars

void gfx_draw_char(int x, int y, char c, uint8_t color);
void gfx_draw_string(int x, int y, const char* str, uint8_t color);

#endif