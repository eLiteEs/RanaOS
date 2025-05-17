#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// Desmaska la línea IRQ 'irq' (0–7 master, 8–15 slave)
void enable_irq(uint8_t irq);

// Remapea los PICs maestro/esclavo a 0x20–0x2F
void remap_pic();

#endif // PIC_H

