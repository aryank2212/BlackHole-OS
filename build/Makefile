# ============================================================================
# BlackHole OS — Makefile
# ============================================================================
# Prerequisites: nasm, i686-elf-gcc (or gcc with -m32), ld, qemu-system-i386
# Usage:
#   make          — build os-image.bin
#   make run      — build and run in QEMU
#   make clean    — remove build artifacts
# ============================================================================

# --- Toolchain ---
ASM      = nasm
CC       = gcc
LD       = ld

# Use cross-compiler if available, else fall back to host gcc -m32
# To use a cross-compiler, set: make CC=i686-elf-gcc LD=i686-elf-ld
CFLAGS   = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin \
           -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c
LDFLAGS  = -m elf_i386 -T ../build/linker.ld --oformat binary

# --- Directories ---
BOOT_DIR   = ../boot
KERNEL_DIR = ../kernel
BUILD_DIR  = .

# --- Source files ---
BOOT_SRC     = $(BOOT_DIR)/boot.asm
KENTRY_SRC   = $(KERNEL_DIR)/kernel_entry.asm
KERNEL_SRC   = $(KERNEL_DIR)/kernel.c

# --- Output files ---
BOOT_BIN     = boot.bin
KENTRY_OBJ   = kernel_entry.o
KERNEL_OBJ   = kernel.o
KERNEL_BIN   = kernel.bin
OS_IMAGE     = os-image.bin

# ============================================================================
# Targets
# ============================================================================

all: $(OS_IMAGE)

# Assemble bootloader (512-byte boot sector)
$(BOOT_BIN): $(BOOT_SRC)
	$(ASM) -f bin $< -o $@

# Assemble kernel entry stub
$(KENTRY_OBJ): $(KENTRY_SRC)
	$(ASM) -f elf32 $< -o $@

# Compile kernel C code
$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(KENTRY_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# Concatenate boot sector + kernel = OS image
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $@

# Run in QEMU
run: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE)

# Clean build artifacts
clean:
	rm -f $(BOOT_BIN) $(KENTRY_OBJ) $(KERNEL_OBJ) $(KERNEL_BIN) $(OS_IMAGE)

.PHONY: all run clean
