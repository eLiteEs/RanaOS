#include "io.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

extern "C" {

    void wait_ms(uint32_t ms) {
        if (ms == 0) return;
    
        // Each iteration is limited to 54.925 ms (maximum divisor = 65535)
        while (ms > 0) {
            uint32_t chunk = (ms > 54) ? 54 : ms;
            ms -= chunk;
    
            uint16_t divisor = (uint16_t)(1193182 / 1000 * chunk); // = 1193 * chunk
    
            // Set PIT channel 0 to mode 0 (one-shot), binary counting
            outb(PIT_COMMAND, 0b00110100); // channel 0, access lobyte/hibyte, mode 0
    
            // Load divisor
            outb(PIT_CHANNEL0, divisor & 0xFF);        // low byte
            outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF); // high byte
    
            // Wait until the countdown is done (OUT == 1)
            while (true) {
                outb(PIT_COMMAND, 0xE2); // latch status of channel 0
                uint8_t status = inb(PIT_CHANNEL0);
                if (status & (1 << 7)) break; // OUT = 1, finished
            }
        }
    }

} // extern "C"