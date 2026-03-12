; ============================================================================
; Kernel Entry Point (ASM stub)
; Called by the bootloader after switching to 32-bit protected mode.
; Calls kernel_main() defined in kernel.c
;
; GCC on Windows prefixes C symbols with underscore.
; NASM win32 does NOT auto-prefix.
; So we use _kernel_main explicitly to match GCC.
; ============================================================================

[bits 32]

extern _kernel_main

global __start

__start:
    call _kernel_main
    jmp $                      ; halt if kernel_main ever returns
