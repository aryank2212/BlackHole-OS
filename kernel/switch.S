[bits 32]
section .text
global _switch_task

; void switch_task(uint32_t *old_esp, uint32_t new_esp);
_switch_task:
    ; Standard C calling convention: parameters are on stack
    ; [esp+4] = old_esp pointer
    ; [esp+8] = new_esp integer
    
    ; 1. Preserve the non-volatile registers on the CURRENT task's stack
    push ebp
    push ebx
    push esi
    push edi

    ; 2. Grab the pointer to old_esp
    mov eax, [esp+20]  ; 4 pushes (16 bytes) + original 4-byte offset = 20

    ; 3. Save the *current* stack pointer inside old_task->esp
    mov [eax], esp

    ; 4. Grab the *new* stack pointer value
    mov esp, [esp+24]  ; Next parameter is at +24

    ; 5. Restore the non-volatile registers from the NEW task's stack
    pop edi
    pop esi
    pop ebx
    pop ebp

    ; 6. Return physically jumps into the new task's execution line (because its EIP was pushed onto the top of its stack!)
    ret
