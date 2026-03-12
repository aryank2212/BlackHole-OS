/* ============================================================================
 * Programmable Interval Timer (PIT) Driver — BlackHole OS
 *
 * Configures the 8253/8254 PIT on IRQ0 to track system uptime and provide
 * sleep functionalities.
 * ========================================================================= */

#include "timer.h"
#include "../kernel/idt.h" /* For outb/inb */
#include "../kernel/isr.h" /* To register handler if needed, though we hardcode in isr.c */
#include "../kernel/task.h" /* Scheduler Context Switcher */

static uint32_t tick = 0;
static uint32_t current_freq = 100;

/* Called by the IRQ0 interrupt handler */
void timer_callback(void) {
    tick++;
    /* Trigger the Multitasking Scheduler at every 10ms tick! */
    task_switch();
}

void timer_init(uint32_t frequency) {
    current_freq = frequency;
    
    /* 
     * The PIT's internal clock runs at 1193182 Hz.
     * We calculate the required divisor to get the desired frequency.
     */
    uint32_t divisor = 1193182 / frequency;

    /*
     * Command port 0x43 configuration:
     * - Bit 6-7: Channel 0 (00)
     * - Bit 4-5: Access mode: lobyte/hibyte (11)
     * - Bit 1-3: Operating mode 3 - Square wave generator (011)
     * - Bit 0: 16-bit binary (0)
     * 00110110 = 0x36
     */
    outb(0x43, 0x36);
    
    /* Send the divisor byte by byte (lobtye then hibyte) to data port 0x40 */
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return tick;
}

void sleep(uint32_t ms) {
    /* 
     * If frequency is 100Hz, 1 tick = 10ms.
     * 1000 ms / current_freq = ms_per_tick.
     */
    uint32_t ticks_to_wait = ms / (1000 / current_freq);
    uint32_t end = tick + ticks_to_wait;
    
    /* Busy wait until the tick reaches the calculated end time */
    while (tick < end) {
        /* wait via __asm__("hlt"); ?
         * Since taking interrupts, HLT is safe here. 
         * It halts CPU until next interrupt fires.
         */
        __asm__ __volatile__("hlt");
    }
}
