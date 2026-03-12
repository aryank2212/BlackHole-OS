/* ============================================================================
 * ISR — Interrupt Service Routine stubs (GAS syntax)
 *
 * Using GAS (.S) compiled by GCC ensures consistent symbol mangling
 * with the rest of the C code (all symbols get _ prefix on Windows).
 * ========================================================================= */

.code32

.extern _irq1_handler
.global _isr_irq1

_isr_irq1:
    pusha
    call _irq1_handler
    popa
    iret
