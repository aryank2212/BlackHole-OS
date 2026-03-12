#ifndef TASK_H
#define TASK_H

#include "../include/stdint.h"

/* Structure representing a running execution thread */
typedef struct task {
    uint32_t id;                /* Process ID */
    uint32_t esp;               /* Stack Pointer */
    uint32_t ebp;               /* Base Pointer */
    uint32_t eip;               /* Instruction Pointer */
    uint32_t cr3;               /* Page Directory (Physical Address) */
    struct task *next;          /* Linked list pointer to the next task */
} task_t;

/* Initialize the multitasking system */
void tasking_init(void);

/* Yield the current thread and switch to the next one */
void task_switch(void);

/* Spawn a new kernel thread spinning in parallel */
void create_kernel_thread(void (*entry_point)(void));

#endif /* TASK_H */
