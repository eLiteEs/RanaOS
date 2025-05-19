ENTRY(_start)

SECTIONS {
    . = 1M;

    .text : {
        *(.multiboot)
        *(.text*)
        *(.iplt*)       /* Algunas versiones de g++ agregan esto */
        *(.eh_frame)    /* Mismo caso, necesario para algunos entornos */
        *(.rel.dyn)     /* A veces aparece, lo metemos por seguridad */
    } :text

    .rodata : {
        *(.rodata*)
    } :text

    .data : {
        *(.data*)
        *(.got*)
        *(.got.plt*)
        *(.igot.plt*)
    } :data

    .bss : {
        *(.bss*)
    } :nobits
}

PHDRS {
    text PT_LOAD FLAGS(5);  /* R + X == 0x1 | 0x4 = 0x5 */
    data PT_LOAD FLAGS(6);  /* R + W == 0x2 | 0x4 = 0x6 */
    nobits PT_LOAD FLAGS(6);
}

