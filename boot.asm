[bits 32]
section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002
    dd 0x00
    dd - (0x1BADB002 + 0x00)

section .text
global _start
extern kmain

_start:
    call kmain
.halt:
    jmp .halt

