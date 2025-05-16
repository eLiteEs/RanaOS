[BITS 32]
[GLOBAL _start]
[EXTERN kmain]

section .multiboot
align 4
    dd 0x1BADB002             ; magic
    dd 0x00                   ; flags
    dd -(0x1BADB002 + 0x00)   ; checksum

section .text
_start:
    cli
    mov esp, stack_top
    call kmain

.hang:
    jmp .hang

section .bss
align 4
stack_bottom:
    resb 4096
stack_top:

