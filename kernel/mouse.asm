[bits 32]
global mouse_handler_asm

mouse_handler_asm:
    pushad
    cld
    call handle_ps2_mouse
    popad
    iretd