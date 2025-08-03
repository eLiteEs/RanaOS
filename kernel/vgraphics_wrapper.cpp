// kernel/vgraphics_wrapper.cpp

#include "vgraphics.h"

extern "C" {

void vgraphics_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    VGraphics::putPixel(x, y, color);
}

void vgraphics_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    VGraphics::drawChar(x, y, c, fg, bg);
}

void vgraphics_clear_screen() {
    VGraphics::clearScreen();
}

uint32_t vgraphics_get_width() {
    return VGraphics::getWidth();
}

uint32_t vgraphics_get_height() {
    return VGraphics::getHeight();
}

void vgraphics_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg) {
    VGraphics::drawString(x, y, str, fg);
}

}
