;
; boot.asm
; Load framebuffer info to memory
; Call the kernel.cpp kmain function
; IMPORTANT: kmain should not return, it would freeze the computer
; Compile with: nasm -f elf32 boot.asm -o boot.o
;

section .multiboot2
align 8
header_start:
    dd 0xE85250D6             ; magic
    dd 0                      ; architecture (i386)
    dd header_end - header_start
    dd -(0xE85250D6 + 0 + (header_end - header_start))

align 8
    dw 5                      ; framebuffer tag type
    dw 0                      ; flags
    dd 24                     ; size of tag
    dd 1024                   ; width
    dd 768                    ; height
    dd 32                     ; bpp
    dd 0                      ; padding to reach 24 bytes

align 8
    dw 0                      ; end tag
    dw 0
    dd 8
header_end:

section .text
global _start
extern kmain

_start:
    mov esp, stack_top
    push ebx
    push eax
    call kmain
    cli
.hang:  hlt
        jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
