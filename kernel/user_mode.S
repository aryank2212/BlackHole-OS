[bits 32]
section .text
global _switch_to_user_mode

_switch_to_user_mode:
    cli                         ; Pause interrupts during the switch

    ; Set up the segment registers for Ring 3 Data Segment
    ; The User Data Segment is descriptor 4 in our GDT.
    ; Its offset is 4 * 8 = 32 (0x20).
    ; We OR it with 3 (RPL 3) to request Ring 3 execution -> 0x20 | 0x3 = 0x23
    mov ax, 0x23 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push the fake iret stack frame
    ; When 'iret' is executed, the CPU expects:
    ; SS, ESP, EFLAGS, CS, EIP

    ; 1. Push SS (Ring 3 Data Segment)
    mov eax, 0x23
    push eax

    ; 2. Push ESP (Current stack pointer, just keeping it simple for the example)
    mov eax, esp
    push eax

    ; 3. Push EFLAGS (Enable Interrupts inside User Mode by setting bit 9)
    pushf
    pop eax
    or eax, 0x200
    push eax

    ; 4. Push CS (Ring 3 Code Segment)
    ; User Code Segment is descriptor 3 -> 3 * 8 = 24 (0x18). RPL 3 -> 0x1B
    mov eax, 0x1B
    push eax

    ; 5. Push EIP (where the CPU returns AFTER iret)
    ; We need an external label in our C code, e.g., _user_main
    extern _user_main
    push _user_main

    ; Perform the magic
    iret

    ; The CPU will never reach here, it will physically leap to _user_main in Ring 3.
