#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "Font.h"

// Colores VGA estándar
enum {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_BROWN,
    COLOR_LIGHT_GRAY,
    COLOR_DARK_GRAY,
    COLOR_LIGHT_BLUE,
    COLOR_LIGHT_GREEN,
    COLOR_LIGHT_CYAN,
    COLOR_LIGHT_RED,
    COLOR_LIGHT_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE
};

// Constantes matemáticas
#define PI 3.141592653589793f
#define DEG_TO_RAD (PI / 180.0f)

// Estructuras para gráficos 3D
/*struct Vec3 {
    float x, y, z;
};

struct Triangle {
    Vec3 v0, v1, v2;
    uint8_t color;
};*/

// Funciones básicas
void gfx_init();
void gfx_switch_to_text();
#ifdef __cplusplus
extern "C" {
#endif

void gfx_put_pixel(int x, int y, uint8_t color);
// Otras funciones que necesites desde C

#ifdef __cplusplus
}
#endif
void gfx_clear_screen(uint8_t color);

// Formas 2D
void gfx_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void gfx_draw_rect(int x, int y, int w, int h, uint8_t color);
void gfx_fill_rect(int x, int y, int w, int h, uint8_t color);
void gfx_draw_circle(int cx, int cy, int r, uint8_t color);
void gfx_fill_circle(int cx, int cy, int r, uint8_t color);

// Texto
/*void gfx_set_font(uint8_t* font_data);
void gfx_draw_char(int x, int y, char c, uint8_t color);
void gfx_draw_string(int x, int y, const char* str, uint8_t color);

// Funciones 3D
/*void gfx_draw_triangle(const Triangle& tri);
void gfx_draw_mesh(const Triangle* triangles, int count);
void gfx_set_projection(float fov, float aspect, float near, float far);*/
void gfx_set_camera(float x, float y, float z, float rx, float ry, float rz);

// Threads (sistema básico de multitarea)
typedef void (*ThreadFunc)(void*);
int thread_create(ThreadFunc func, void* arg);
void thread_yield();
void thread_sleep(int ms);

#endif