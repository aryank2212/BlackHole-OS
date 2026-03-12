/* ============================================================================
 * ISR — Interrupt Service Routine stubs (GAS syntax)
 *
 * Using GAS (.S) compiled by GCC ensures consistent symbol mangling
 * with the rest of the C code (all symbols get _ prefix on Windows).
 * ========================================================================= */

.code32

.extern _isr14_handler
.extern _irq0_handler
.extern _irq1_handler
.extern _isr128_handler

.global _isr_14
.global _isr_irq0
.global _isr_irq1
.global _isr_128

_isr_14:
    pusha
    call _isr14_handler
    popa
    add $4, %esp    /* Clean up the hardware-pushed error code */
    iret

_isr_irq0:
    pusha
    call _irq0_handler
    popa
    iret

_isr_irq1:
    pusha
    call _irq1_handler
    popa
    iret

/* Syscall Handler (INT 0x80) */
_isr_128:
    cli
    pusha
    
    /* Push arguments for the C function: arg2 (ecx), arg1 (ebx), syscall_num (eax) */
    push %ecx
    push %ebx
    push %eax
    
    call _isr128_handler
    
    add $12, %esp  /* Clean up 3 * 4 bytes arguments */
    
    popa
    sti
    iret
