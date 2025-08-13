ENTRY(_start)

SECTIONS {
    . = 1M;

    .text : {
        KEEP(*(.multiboot2))   /* Header Multiboot2 debe ir primero */
        *(.text*)
        *(.iplt*)              
        *(.eh_frame)           
        *(.rel.dyn)
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
    text PT_LOAD FLAGS(5);   /* RX */
    data PT_LOAD FLAGS(6);   /* RW */
    nobits PT_LOAD FLAGS(6); /* RW, BSS */
}
