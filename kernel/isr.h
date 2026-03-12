#ifndef ISR_H
#define ISR_H

#include "../include/stdint.h"

/* ASM-defined IRQ stub (in isr_stub.S) */
extern void isr_14(void);
extern void isr_irq0(void);
extern void isr_irq1(void);
extern void isr_128(void);

/* C handler called from the ASM stub */
void isr14_handler(void);
void irq0_handler(void);
void irq1_handler(void);
void isr128_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2);

#endif /* ISR_H */
