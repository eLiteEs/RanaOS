; exceptions.asm - Manejadores completos de excepciones e interrupciones
[bits 32]

; =============================================
; Macros para definición de handlers
; =============================================

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push byte 0         ; Push dummy error code
    push byte %1        ; Push interrupt number
    jmp isr_common      ; Saltar al manejador común
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push byte %1        ; Push interrupt number (error code ya está en stack)
    jmp isr_common
%endmacro

; =============================================
; Definición de todos los manejadores de excepciones (0-31)
; =============================================

ISR_NOERRCODE 0    ; Divide By Zero
ISR_NOERRCODE 1    ; Debug
ISR_NOERRCODE 2    ; Non Maskable Interrupt
ISR_NOERRCODE 3    ; Breakpoint
ISR_NOERRCODE 4    ; Overflow
ISR_NOERRCODE 5    ; Bound Range Exceeded
ISR_NOERRCODE 6    ; Invalid Opcode
ISR_NOERRCODE 7    ; Device Not Available
ISR_ERRCODE   8    ; Double Fault
ISR_NOERRCODE 9    ; Coprocessor Segment Overrun
ISR_ERRCODE   10   ; Invalid TSS
ISR_ERRCODE   11   ; Segment Not Present
ISR_ERRCODE   12   ; Stack Segment Fault
ISR_ERRCODE   13   ; General Protection Fault
ISR_ERRCODE   14   ; Page Fault
ISR_NOERRCODE 15   ; Reserved
ISR_NOERRCODE 16   ; x87 Floating-Point Exception
ISR_ERRCODE   17   ; Alignment Check
ISR_NOERRCODE 18   ; Machine Check
ISR_NOERRCODE 19   ; SIMD Floating-Point Exception
ISR_NOERRCODE 20   ; Virtualization Exception
ISR_NOERRCODE 21   ; Control Protection Exception
; 22-31: Reservados
%assign i 22
%rep 10
ISR_NOERRCODE i
%assign i i+1
%endrep

; =============================================
; Tabla de stubs para la IDT (SOLO UNA DEFINICIÓN)
; =============================================
global isr_stub_table
isr_stub_table:
%assign i 0
%rep 32
    dd isr%+i         ; Punteros a todos los ISRs
%assign i i+1
%endrep

; =============================================
; Manejador común de excepciones
; =============================================
extern isr_handler    ; Definido en idt.cpp

isr_common:
    ; Guardar todos los registros
    pusha             ; EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    
    ; Guardar segmentos de datos
    push ds
    push es
    push fs
    push gs
    
    ; Cargar segmento de kernel (0x10 es el selector de datos del GDT)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Preparar puntero a la estructura de registros
    push esp
    
    ; Llamar al manejador en C
    call isr_handler
    
    ; Limpiar el parámetro de la pila
    add esp, 4
    
    ; Restaurar segmentos
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restaurar registros generales
    popa
    
    ; Limpiar error code y número de interrupción
    add esp, 8
    
    ; Retornar de la interrupción
    iret

; =============================================
; Manejadores de IRQs (32-47)
; =============================================
%macro IRQ 2
global irq%1
irq%1:
    push byte 0      ; Error code dummy
    push byte %2     ; Número de interrupción
    jmp irq_common
%endmacro

; Definición de IRQs
IRQ 0, 32    ; Timer
IRQ 1, 33    ; Keyboard
IRQ 2, 34    ; Cascade (never raised)
IRQ 3, 35    ; COM2
IRQ 4, 36    ; COM1
IRQ 5, 37    ; LPT2
IRQ 6, 38    ; Floppy
IRQ 7, 39    ; LPT1
IRQ 8, 40    ; CMOS RTC
IRQ 9, 41    ; Free
IRQ 10, 42   ; Free
IRQ 11, 43   ; Free
IRQ 12, 44   ; PS2 Mouse
IRQ 13, 45   ; FPU
IRQ 14, 46   ; Primary ATA
IRQ 15, 47   ; Secondary ATA

; Manejador común para IRQs
extern irq_handler

irq_common:
    ; Guardar registros (igual que en isr_common)
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Segmentos de kernel
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Llamar al manejador C
    push esp
    call irq_handler
    add esp, 4
    
    ; Restaurar todo
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    ; Limpiar stack
    add esp, 8
    
    ; Retornar
    iret

; =============================================
; Exportar tamaños para verificación
; =============================================
global isr_stub_table_size
isr_stub_table_size: dd 32    ; Tamaño de la tabla de excepciones