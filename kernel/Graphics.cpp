#include "Graphics.h"
#include "io.h"
#include "Font.h"
#include "math.hpp"
#include "string.h"
#include <stdbool.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

static uint8_t* vga = (uint8_t*)0xA0000;
static uint8_t* font = nullptr;

// Implementación de funciones matemáticas básicas
static float absf(float x) { return (x < 0) ? -x : x; }
static int absi(int x) { return (x < 0) ? -x : x; }
static float sinf(float x);
static float cosf(float x);
static float tanf(float x);

// ================== FUNCIONES BÁSICAS ==================
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

void write_registers(uint8_t* regs) {
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

void gfx_init() {
    write_registers(g_320x200x256);
    
    // Configurar plano de memoria
    outb(0x3C4, 0x02);
    outb(0x3C5, 0x0F);
}

void gfx_put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    vga[y * SCREEN_WIDTH + x] = color;
}

void gfx_clear_screen(uint8_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga[i] = color;
    }
}

// ================== FORMAS 2D ==================
void gfx_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = absi(x1 - x0);
    int dy = -absi(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    
    while (true) {
        gfx_put_pixel(x0, y0, color);
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

void gfx_draw_rect(int x, int y, int w, int h, uint8_t color) {
    gfx_draw_line(x, y, x + w, y, color);
    gfx_draw_line(x + w, y, x + w, y + h, color);
    gfx_draw_line(x + w, y + h, x, y + h, color);
    gfx_draw_line(x, y + h, x, y, color);
}

void gfx_fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            gfx_put_pixel(x + dx, y + dy, color);
        }
    }
}

void gfx_draw_circle(int cx, int cy, int r, uint8_t color) {
    int x = r;
    int y = 0;
    int decision = 1 - x;
    
    while (x >= y) {
        gfx_put_pixel(cx + x, cy + y, color);
        gfx_put_pixel(cx - x, cy + y, color);
        gfx_put_pixel(cx + x, cy - y, color);
        gfx_put_pixel(cx - x, cy - y, color);
        gfx_put_pixel(cx + y, cy + x, color);
        gfx_put_pixel(cx - y, cy + x, color);
        gfx_put_pixel(cx + y, cy - x, color);
        gfx_put_pixel(cx - y, cy - x, color);
        
        y++;
        if (decision <= 0) {
            decision += 2 * y + 1;
        } else {
            x--;
            decision += 2 * (y - x) + 1;
        }
    }
}

void gfx_fill_circle(int cx, int cy, int r, uint8_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                gfx_put_pixel(cx + x, cy + y, color);
            }
        }
    }
}
void gfx_put_image(int dx, int dy, const char* img_data) {
    int x = dx;
    int y = dy;
    const int max_width = 320;
    const int max_height = 200;
    bool rendering = true;

    while (rendering && y < max_height) {
        char c = *img_data++;
        
        switch (c) {
            case 'n': case 'b': case 'g': case 'c': 
            case 'r': case 'm': case 'l': case 'y': case 'w':
                if (x < max_width && y >= 0 && y < max_height) {
                    uint8_t color;
                    switch (c) {
                        case 'n': color = COLOR_BLACK; break;
                        case 'b': color = COLOR_BLUE; break;
                        case 'g': color = COLOR_GREEN; break;
                        case 'c': color = COLOR_CYAN; break;
                        case 'r': color = COLOR_RED; break;
                        case 'm': color = COLOR_MAGENTA; break;
                        case 'l': color = COLOR_BROWN; break;
                        case 'y': color = COLOR_YELLOW; break;
                        case 'w': color = COLOR_WHITE; break;
                    }
                    gfx_put_pixel(x, y, color);
                }
                x++;
                break;
            
            case '\n':
                x = dx;
                y++;
                break;
            
            case 'q':
            case '\0':
                rendering = false;
                break;
                
            default:
                gfx_put_pixel(x, y, COLOR_RED);
                x++;
                break;
        }
    }
}

// ================== GRÁFICOS 3D ==================
static float projection_matrix[4][4] = {{0}};
static float camera_matrix[4][4] = {{0}};

void gfx_set_projection(float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov / 2.0f);
    
    projection_matrix[0][0] = f / aspect;
    projection_matrix[1][1] = f;
    projection_matrix[2][2] = (far + near) / (near - far);
    projection_matrix[2][3] = -1.0f;
    projection_matrix[3][2] = (2.0f * far * near) / (near - far);
}

void gfx_set_camera(float x, float y, float z, float rx, float ry, float rz) {
    // Implementación básica (sin rotaciones)
    camera_matrix[3][0] = -x;
    camera_matrix[3][1] = -y;
    camera_matrix[3][2] = -z;
}

static Vec3 transform_point(const Vec3& point) {
    // Aplicar transformaciones de cámara
    Vec3 transformed = {
        point.x + camera_matrix[3][0],
        point.y + camera_matrix[3][1],
        point.z + camera_matrix[3][2]
    };
    
    // Aplicar proyección
    float w = projection_matrix[3][2] / transformed.z;
    Vec3 projected = {
        transformed.x * w * projection_matrix[0][0],
        transformed.y * w * projection_matrix[1][1],
        transformed.z
    };
    
    // Convertir a coordenadas de pantalla
    Vec3 screen = {
        (projected.x + 1.0f) * 0.5f * SCREEN_WIDTH,
        (1.0f - projected.y) * 0.5f * SCREEN_HEIGHT,
        projected.z
    };
    
    return screen;
}

void gfx_draw_triangle(const Triangle& tri) {
    Vec3 v0 = transform_point(tri.v0);
    Vec3 v1 = transform_point(tri.v1);
    Vec3 v2 = transform_point(tri.v2);
    
    // Dibujar líneas del triángulo
    gfx_draw_line((int)v0.x, (int)v0.y, (int)v1.x, (int)v1.y, tri.color);
    gfx_draw_line((int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, tri.color);
    gfx_draw_line((int)v2.x, (int)v2.y, (int)v0.x, (int)v0.y, tri.color);
}

void gfx_draw_mesh(const Triangle* triangles, int count) {
    for (int i = 0; i < count; i++) {
        gfx_draw_triangle(triangles[i]);
    }
}

// ================== THREADS ==================
/*struct Thread {
    void* stack;
    void* stack_ptr;
    bool active;
};

#define MAX_THREADS 8
static Thread threads[MAX_THREADS];
static int current_thread = 0;

// Interrupt handler para cambio de contexto
extern "C" void thread_switch_asm(void** old_sp, void* new_sp);

int thread_create(ThreadFunc func, void* arg) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (!threads[i].active) {
            // Crear pila de 4KB
            threads[i].stack = new uint8_t[4096];
            threads[i].stack_ptr = (void*)((uint8_t*)threads[i].stack + 4096 - 16);
            
            // Configurar contexto inicial
            uint32_t* stack = (uint32_t*)threads[i].stack_ptr;
            *(--stack) = (uint32_t)arg;  // Argumento
            *(--stack) = 0;              // Registro EBP
            *(--stack) = 0;              // Registro EBX
            *(--stack) = 0;              // Registro ESI
            *(--stack) = 0;              // Registro EDI
            *(--stack) = (uint32_t)func; // Dirección de retorno (punto de entrada)
            
            threads[i].stack_ptr = stack;
            threads[i].active = true;
            return i;
        }
    }
    return -1;
}

void thread_yield() {
    int next_thread = (current_thread + 1) % MAX_THREADS;
    while (!threads[next_thread].active) {
        next_thread = (next_thread + 1) % MAX_THREADS;
    }
    
    void* old_sp;
    asm volatile("mov %%esp, %0" : "=r"(old_sp));
    
    threads[current_thread].stack_ptr = old_sp;
    current_thread = next_thread;
    
    thread_switch_asm(&old_sp, threads[current_thread].stack_ptr);
}

void thread_sleep(int ms) {
    // Implementación básica con busy-wait
    // (En un sistema real usaríamos el PIT)
    volatile int count = ms * 10000;
    while (count-- > 0) {
        asm volatile("pause");
    }
}*/

// Implementación de funciones trigonométricas básicas
static float sinf(float x) {
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

static float cosf(float x) {
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
