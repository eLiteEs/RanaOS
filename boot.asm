; boot.asm
section .multiboot
header_start:
    dd 0x1BADB002              ; Magic number (Multiboot 1)
    dd 0x00000003              ; Flags (align modules + provide memory map)
    dd -(0x1BADB002 + 0x00000003) ; Checksum
header_end:

section .text
global _start
extern kmain

_start:
    mov esp, stack_top         ; Set up stack
    push ebx                   ; Push Multiboot info struct pointer
    push eax                   ; Push Multiboot magic number
    call kmain                 ; Call your kernel main
    cli
.hang: hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384                 ; 16KB stack
stack_top: