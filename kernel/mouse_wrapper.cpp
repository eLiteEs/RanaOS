#include "Mouse.h"

extern "C" {

void mouse_initialize() {
    Mouse::initialize();
}

int16_t mouse_get_x() {
    return Mouse::get_x();
}

int16_t mouse_get_y() {
    return Mouse::get_y();
}

uint8_t mouse_get_buttons() {
    return Mouse::get_buttons();
}

}