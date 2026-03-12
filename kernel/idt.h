#ifndef IDT_H
#define IDT_H

/* Port I/O helpers */
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(unsigned short port, unsigned short val) {
    __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned short inw(unsigned short port) {
    unsigned short ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* IDT entry (8 bytes) */
typedef struct {
    unsigned short offset_low;    /* offset bits 0..15  */
    unsigned short selector;      /* code segment selector */
    unsigned char  zero;          /* unused, set to 0 */
    unsigned char  type_attr;     /* type and attributes */
    unsigned short offset_high;   /* offset bits 16..31 */
} __attribute__((packed)) idt_entry_t;

/* IDT pointer for lidt instruction */
typedef struct {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed)) idt_ptr_t;

/* Number of IDT entries (32 exceptions + 16 IRQs) */
#define IDT_ENTRIES 256

/* Set a single IDT gate */
void idt_set_gate(int n, unsigned int handler, unsigned short sel, unsigned char flags);

/* Initialize the IDT and remap the PIC */
void idt_init(void);

#endif /* IDT_H */
