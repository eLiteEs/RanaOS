#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

class Keyboard {
    public:
        static bool was_c_pressed();
        static bool was_key_pressed(char key);
    private:
        static int keyboard_key_available();
        static uint8_t keyboard_read_scancode();
};

#endif // KEYBOARD_H
