# BlackHole OS

**BlackHole OS** is a minimal, experimental x86 operating system built from scratch to explore OS design principles, minimalist architecture, and modular development.

## 🚀 Features (v0.1.0)

* **Custom Bootloader:** A two-stage x86 bootloader (MBR) written in NASM that loads the kernel from disk and enters 32-bit Protected Mode.
* **Minimal C Kernel:** A lightweight 32-bit kernel built with MinGW GCC.
* **VGA Text Driver:** Full 16-color VGA text mode support, featuring cursor tracking, logical scrolling, and formatted character printing.
* **Interrupt handling (IDT & ISR):** Programmed the 8259 PIC and initialized the Interrupt Descriptor Table (IDT) for managing hardware interrupts.
* **PS/2 Keyboard Driver:** A ring-buffered keyboard driver running off IRQ1, supporting US QWERTY layouts with shift-key and essential backspace handling.
* **Memory Management:** A custom free-list heap allocator mapping out memory at physical address `1MB` (`0x100000`) capable of allocation (`alloc`), splitting, and coalescing.
* **Interactive Shell:** A responsive command shell (`BH-OS>`) running in an infinite loop, capable of parsing commands like `help`, `echo`, `clear`, and `alloc`.

## 🛠️ System Architecture

```text
BIOS → Bootloader (Stage 1 & 2) → Protected Mode Kernel → Drivers (VGA, Keyboard) → Memory Allocator → Interactive Shell
```

## 🏗️ Building and Running locally (Windows)

The OS is compiled, linked, and assembled purely locally utilizing **NASM** and **MinGW** (no WSL required). QEMU is run to emulate the x86 environment.

**Prerequisites:**
You will need PowerShell, [NASM](https://nasm.us/), [MinGW-w64](https://www.mingw-w64.org/), and [QEMU](https://www.qemu.org/). If you don't have them, the build script can optionally install them via Chocolatey contextually.

**To Build & Run:**
```powershell
cd build
.\build.ps1 clean
.\build.ps1 run
```

This automates the assembly, C compilation (`-m32 -ffreestanding`), PE linking (`i386pe`), flat binary conversion (`objcopy`), and `os-image.bin` construction, ultimately booting into QEMU!

## 📜 Development Philosophy
1. Minimalism. No unnecessary components or bloated drivers.
2. Modular architecture. Clean separation between boot, kernel, drivers, and shell.
3. Native Windows Development flow.

---

*Note: This repository was repurposed from a previous framework project. The current `main` branch solely contains the BlackHole OS system codebase.*
