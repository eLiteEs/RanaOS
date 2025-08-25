#include "Keyboard.h"

extern "C" {

bool was_c_pressed() {
    return Keyboard::was_c_pressed();
}

bool was_key_pressed(char key) {
    return Keyboard::was_key_pressed(key);
}

char key_pressed() {
    return Keyboard::key_pressed();
}

}
