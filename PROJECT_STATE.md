# BLACKHOLE OS PROJECT STATE

**Project Name:** BlackHole  
**Version:** 0.1.1  
**Architecture:** x86 (initial version)

**Goal:**  
Create a minimal experimental operating system that boots,
loads a kernel, manages memory, and provides an interactive environment.

---

## SYSTEM ARCHITECTURE

```text
Bootloader → Protected Mode Kernel → Drivers (VGA, Keyboard, PIT Timer) → Standard Library (libc) → Memory Allocator → Interactive Shell
```

---

## DIRECTORY STRUCTURE

```text
/boot
  boot.asm              ← x86 bootloader (MBR)

/kernel
  kernel_entry.asm      ← ASM stub calling kernel_main
  kernel.c              ← Kernel entry point
  idt.h / idt.c         ← Interrupt Descriptor Table + PIC
  isr.h / isr.c         ← ISR C-side handlers
  isr_stub.S            ← IRQ ASM stubs (GAS format)
  memory.h / memory.c   ← Memory allocator (Free-list)

/drivers
  vga.h / vga.c         ← VGA text mode driver
  keyboard.h / keyboard.c ← Keyboard driver (IRQ1)
  timer.h / timer.c     ← PIT Timer driver (IRQ0)

/libc
  string.c              ← memset, memcpy, strlen, strcmp, strncmp, itoa
  stdio.c               ← printf, puts, putchar

/include
  stdint.h, stdbool.h, stdarg.h, string.h, stdio.h  ← Standard headers

/shell
  shell.h / shell.c     ← Interactive shell

/build
  build.ps1             ← PowerShell build script (Windows)
```

---

## RECENT UPGRADES (Phase 2)

1. **Standard Library (`libc`)**:
   - Implemented `<stdint.h>`, `<stdbool.h>`, `<stdarg.h>`, `<string.h>`, and `<stdio.h>`.
   - Built a custom, fully functional `printf` routine utilizing variable arguments targeting the VGA display.
   - Refactored the core Kernel and Shell to exclusively use `printf` and `strcmp` string manipulations natively.

2. **Programmable Interval Timer (PIT)**:
   - Initialized the Intel 8253 PIT chip on IRQ0 running at 100Hz.
   - Wired the PIT directly into the IDT configuration and the C ISR dispatch tables via Gas assembly `isr_stub.S`.
   - Developed `sleep(ms)` precision timing functionality.
   - Added robust `uptime` and `sleep` testing mechanisms into the user-facing Shell.
