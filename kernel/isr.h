#ifndef ISR_H
#define ISR_H

/* ASM-defined IRQ stub (in isr_stub.S) */
extern void isr_14(void);
extern void isr_irq0(void);
extern void isr_irq1(void);

/* C handler called from the ASM stub */
void isr14_handler(void);
void irq0_handler(void);
void irq1_handler(void);

#endif /* ISR_H */
