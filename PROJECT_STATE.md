# BLACKHOLE OS PROJECT STATE

**Project Name:** BlackHole  
**Version:** 0.1.4  
**Architecture:** x86 (initial version)

**Goal:**  
Create a minimal experimental operating system that boots,
loads a kernel, manages memory, and provides an interactive environment.

---

## SYSTEM ARCHITECTURE

```text
Bootloader → Protected Mode Kernel → User Space (Ring 3) → Syscalls (INT 0x80) → Subsystems (VGA, Keyboard, ATA HDD, Memory Allocator)
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
  paging.h / paging.c   ← Virtual Memory / MMU hardware logic

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

3. **Virtual Memory (Paging)**:
   - Configured Hardware Memory Management Translation across the `CR3` and `CR0` core hardware registers.
   - Identity mapped the lower 4 Megabytes of system memory natively.
   - Registered deep diagnostic logging over Hardware Exception 14 handling safe `kernel_panic` states instead of emulator reboots via fetching data directly from memory state pipeline over `CR2`.

4. **Primary Storage (ATA PIO)**:
   - Added `outw` and `inw` hardware inline assembly definitions.
   - Wrote a full 512-byte blocking sequence disk driver for the Primary IDE master controller via 28-bit LBA (Ports `0x1F0` - `0x1F7`).
   - Integrated a 1MB dynamic raw filesystem image attachment into QEMU via PowerShell utilizing `fsutil`.

5. **User Mode (Ring 3) & Multitasking Prep**:
   - Replaced the bootloader GDT with a fully-featured C Kernel GDT assigning Descriptor Privilege Level 3 to User Application Segments.
   - Connected a Task State Segment (TSS) caching the Ring 0 execution `ESP` registers preventing system stack destruction during Ring 3 interrupts.
   - Setup a `switch_to_user_mode` generic assembly router firing an `iret` payload to strip System Level permissions dynamically.
   - Intercepted Syscall Software Interrupt `0x80` allowing Ring 3 software to communicate with the Ring 0 backend (e.g. `printf`).
