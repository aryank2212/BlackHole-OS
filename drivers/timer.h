#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Initialize the PIT to the given frequency (default is usually 100Hz) */
void timer_init(uint32_t frequency);

/* Get system uptime in timer ticks */
uint32_t timer_get_ticks(void);

/* Busy-wait for 'ms' milliseconds */
void sleep(uint32_t ms);

/* Called by the IRQ0 interrupt handler */
void timer_callback(void);

#endif /* TIMER_H */
