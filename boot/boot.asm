; ============================================================================
; BlackHole OS Bootloader
; Stage 1: MBR (512 bytes) — loads kernel sectors from disk
; Stage 2: Switches to 32-bit protected mode, jumps to kernel at 0x1000
; ============================================================================

[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000
KERNEL_SECTORS equ 15          ; number of disk sectors to load (7.5 KB)

; ------- Stage 1: Real Mode Setup -------
boot_start:
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00             ; stack grows downward from bootloader

    ; Save boot drive number (BIOS puts it in DL)
    mov [BOOT_DRIVE], dl

    ; Print boot message
    mov si, MSG_BOOTING
    call print_string_rm

    ; Load kernel from disk into KERNEL_OFFSET
    call load_kernel

    ; Switch to protected mode
    call switch_to_pm

    jmp $                      ; should never reach here

; ------- Disk Load Routine -------
load_kernel:
    mov si, MSG_LOAD_KERNEL
    call print_string_rm

    mov ah, 0x02               ; BIOS read sectors function
    mov al, KERNEL_SECTORS     ; number of sectors to read
    mov ch, 0                  ; cylinder 0
    mov cl, 2                  ; start from sector 2 (sector 1 = boot sector)
    mov dh, 0                  ; head 0
    mov dl, [BOOT_DRIVE]       ; drive number
    mov bx, KERNEL_OFFSET      ; ES:BX = destination buffer
    int 0x13                   ; BIOS disk interrupt
    jc disk_error              ; jump if carry flag set (error)

    cmp al, KERNEL_SECTORS     ; check sectors actually read
    jne disk_error

    mov si, MSG_LOAD_OK
    call print_string_rm
    ret

disk_error:
    mov si, MSG_DISK_ERR
    call print_string_rm
    jmp $                      ; hang on error

; ------- Real Mode Print (BIOS TTY) -------
print_string_rm:
    pusha
.loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0e
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    popa
    ret

; ------- GDT (Global Descriptor Table) -------
gdt_start:

gdt_null:                      ; mandatory null descriptor
    dd 0x0
    dd 0x0

gdt_code:                      ; code segment: base=0, limit=4GB, exec/read
    dw 0xffff                  ; limit (bits 0–15)
    dw 0x0                     ; base  (bits 0–15)
    db 0x0                     ; base  (bits 16–23)
    db 10011010b               ; access byte: present, ring 0, code, exec/read
    db 11001111b               ; flags + limit (bits 16–19): 4KB gran, 32-bit
    db 0x0                     ; base  (bits 24–31)

gdt_data:                      ; data segment: base=0, limit=4GB, read/write
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b               ; access byte: present, ring 0, data, read/write
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; GDT size
    dd gdt_start               ; GDT address

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ------- Switch to Protected Mode -------
switch_to_pm:
    cli                        ; disable interrupts

    ; Enable A20 line via keyboard controller
    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdt_descriptor]      ; load GDT

    mov eax, cr0
    or eax, 0x1                ; set PE (Protection Enable) bit
    mov cr0, eax

    jmp CODE_SEG:init_pm       ; far jump to flush pipeline + load CS

; ------- 32-bit Protected Mode Entry -------
[bits 32]
init_pm:
    ; Set up segment registers for protected mode
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000           ; set up stack in higher memory
    mov esp, ebp

    ; Jump to kernel entry point
    jmp KERNEL_OFFSET

; ------- Data -------
[bits 16]
BOOT_DRIVE:    db 0
MSG_BOOTING:   db "BlackHole OS Booting...", 13, 10, 0
MSG_LOAD_KERNEL: db "Loading kernel...", 13, 10, 0
MSG_LOAD_OK:   db "Kernel loaded.", 13, 10, 0
MSG_DISK_ERR:  db "Disk read error!", 13, 10, 0

; ------- Boot Sector Padding & Signature -------
times 510 - ($ - $$) db 0
dw 0xaa55
