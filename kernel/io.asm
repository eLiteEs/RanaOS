; io.asm — versión 32 bits

global inb
global outb

; uint8_t inb(uint16_t port)
; Entrada: puerto en [esp + 4]
; Retorno: al

inb:
    push ebp
    mov ebp, esp
    mov dx, [ebp + 8]
    in al, dx
    pop ebp
    ret

; void outb(uint16_t port, uint8_t val)
; Entrada: valor en [esp + 8], puerto en [esp + 4]

outb:
    push ebp
    mov ebp, esp
    mov dx, [ebp + 8]
    mov al, [ebp + 12]
    out dx, al
    pop ebp
    ret

