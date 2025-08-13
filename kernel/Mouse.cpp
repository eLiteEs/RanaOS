#include "Mouse.h"
#include "io.h"
#include "pic.h"

namespace {
    volatile int16_t mouse_x = 160;
    volatile int16_t mouse_y = 100;
    volatile uint8_t mouse_buttons = 0;
    uint16_t screen_width = 320;
    uint16_t screen_height = 200;
}

void Mouse::initialize() {
    // Enable auxiliary device
    outb(0x64, 0xA8);
    
    // Enable interrupts
    outb(0x64, 0x20);
    uint8_t status = (inb(0x60) | 0x02);
    outb(0x64, 0x60);
    outb(0x60, status);
    
    // Enable data reporting
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    
    enable_irq(12); // Unmask IRQ12
}

int16_t Mouse::get_x() {
    return mouse_x;
}

int16_t Mouse::get_y() {
    return mouse_y;
}

uint8_t Mouse::get_buttons() {
    return mouse_buttons;
}

void Mouse::set_boundaries(uint16_t width, uint16_t height) {
    screen_width = width;
    screen_height = height;
}