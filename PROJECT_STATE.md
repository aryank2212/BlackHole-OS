# BLACKHOLE OS PROJECT STATE

**Project Name:** BlackHole  
**Version:** 0.1.0  
**Architecture:** x86 (initial version)

**Goal:**  
Create a minimal experimental operating system that boots,
loads a kernel, manages memory, and provides a simple shell.

**Philosophy:**  
Minimalism. No unnecessary components.

---

## SYSTEM ARCHITECTURE

```
Bootloader → Kernel → Drivers → Shell
```

---

## DIRECTORY STRUCTURE

```
/boot
  boot.asm              ← x86 bootloader (MBR + protected mode switch)

/kernel
  kernel_entry.asm      ← ASM stub calling kernel_main
  kernel.c              ← Kernel entry point (VGA, IDT, Mem, Shell init)
  kernel.h              ← Kernel consts
  idt.h / idt.c         ← Interrupt Descriptor Table + PIC remapping
  isr.h / isr.c         ← ISR C-side handlers
  isr_stub.S            ← IRQ ASM stubs (GAS format)
  memory.h / memory.c   ← Memory allocator (Free-list)

/drivers
  vga.h / vga.c         ← VGA text mode driver
  keyboard.h / keyboard.c ← Keyboard driver (PS/2, US QWERTY)

/shell
  shell.h / shell.c     ← Simple interactive shell interface

/build
  build.ps1             ← PowerShell build script (Windows)
  Makefile              ← GNU Make build (Linux)
  linker.ld             ← Kernel linker script
  install_tools.bat     ← Tool installer (Admin)
```

---

## MODULE STATUS

- [x] Bootloader
- [x] Kernel entry
- [x] VGA text driver
- [x] Keyboard driver
- [x] Memory allocator
- [x] Simple shell

---

## INTERFACES

```c
/* VGA Driver */
vga_init(), vga_clear(), vga_print(), vga_putchar(), vga_set_color() ...

/* Keyboard Driver */
keyboard_init(), keyboard_read(), keyboard_readline(), keyboard_handle_scancode()

/* IDT / Memory */
idt_init(), memory_init(), memory_alloc(), memory_free()

/* Shell */
shell_init(), shell_loop()
```

---

## COMPLETED TASKS

1. **Bootloader** — `boot.asm` loads kernel and enters 32-bit protected mode.
2. **Kernel entry** — `kernel_entry.asm` jumps to C `kernel_main`.
3. **VGA text driver** — `vga.c` handles colored text, scrolling, backspace.
4. **Keyboard driver** — IDT, PIC IRQ1 routing, scancode mapped to ASCII buffer.
5. **Memory allocator** — `memory.c` provides a free-list heap.
6. **Simple Shell** — `shell.c` implements `help`, `echo`, `clear`, and `alloc` commands via `BH-OS>` prompt.

---

## ALL INITIAL OBJECTIVES ACHIEVED!

The core foundation for BlackHole OS v0.1.0 is officially built and functioning.
You can now build upon this for advanced features (e.g. paging, filesystem, processes).
