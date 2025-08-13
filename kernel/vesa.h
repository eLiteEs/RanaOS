#ifndef VESA_H
#define VESA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t rsv_mask;
    uint8_t rsv_position;
    uint8_t directcolor_attributes;
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
};

void init_graphics(struct multiboot_info_t* mbi);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void gclear_screen(uint32_t color);
void draw_square(uint32_t x, uint32_t y, uint32_t size, uint32_t color);
void draw_cursor(uint32_t x, uint32_t y);

#ifdef __cplusplus
}
#endif

#endif // VESA_H