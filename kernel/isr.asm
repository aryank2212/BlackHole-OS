; ============================================================================
; ISR — Interrupt Service Routine stubs (ASM)
;
; These save/restore registers and call C handlers.
; NASM win32 format does NOT auto-prefix underscores.
; GCC -m32 on Windows DOES prefix C symbols with underscore.
; So we must use underscore-prefixed names explicitly.
; ============================================================================

[bits 32]

section .text

extern _irq0_handler
extern _irq1_handler

global _isr_irq0
global _isr_irq1

; IRQ0 — Timer interrupt (PIT)
_isr_irq0:
    pusha
    call _irq0_handler
    popa
    iret

; IRQ1 — Keyboard interrupt
_isr_irq1:
    pusha                ; save all general-purpose registers
    call _irq1_handler   ; call C handler
    popa                 ; restore registers
    iret                 ; return from interrupt
