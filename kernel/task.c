#include "task.h"
#include "memory.h"
#include "paging.h"
#include "user_mode.h"
#include "../include/stdio.h"
#include "../include/string.h"

/* Current task pointer */
volatile task_t *current_task;

/* The head of the task linked list */
volatile task_t *ready_queue;

/* Counter to assign unique simple PIDs */
static uint32_t next_pid = 1;

/* Externally linked assembly routine to physically swap ESP */
extern void switch_task(uint32_t *old_esp, uint32_t new_esp);

/* Fallback dummy to capture current instruction and stack addresses */
extern uint32_t read_eip(void);

/* Read Current Page Directory */
extern uint32_t read_cr3(void);


/* 
 * We use inline assembly to read the current Instruction Pointer 
 * so we can initialize the very first "Main" task tracking correctly.
 */
static __inline__ uint32_t __read_eip(void) {
    uint32_t val;
    __asm__ __volatile__("call 1f \n 1: pop %0" : "=r" (val));
    return val;
}

void tasking_init(void) {
    /* Disable interrupts while we mess with process queues */
    __asm__ __volatile__("cli");

    /* Initialize the very first main kernel task (the one we are running inside right now) */
    current_task = (task_t *)memory_alloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->esp = 0; /* Will be captured dynamically during switch */
    current_task->ebp = 0;
    current_task->eip = 0;
    
    /* Fetch current CR3 via standard inline assembly */
    uint32_t cr3_val;
    __asm__ __volatile__("mov %%cr3, %0" : "=r" (cr3_val));
    current_task->cr3 = cr3_val;
    
    current_task->next = (task_t *)current_task; /* Round Robin links back to itself initially */

    ready_queue = current_task;

    /* Re-enable interrupts */
    __asm__ __volatile__("sti");
}

void task_switch(void) {
    /* If the queue hasn't been initialized or there's only 1 task, do nothing. */
    if (!current_task || current_task->next == current_task) {
        return;
    }

    uint32_t eip = __read_eip();
    
    /* We just woke up! This happens when another thread switches BACK into us.
     * We injected a magical dummy value 0x12345678 into EAX right below here to detect this. 
     * If EAX == 0x12345678, we exit the function immediately and resume normal execution!
     */
    uint32_t magic = 0;
    __asm__ __volatile__("mov %%eax, %0" : "=r" (magic));
    
    if (magic == 0x12345678) {
        return;
    }

    /* We are going TO sleep. Save our state and jump! */
    current_task->eip = eip;

    /* Get the next task */
    task_t *next_task = current_task->next;

    /* Perform the physical assembly switch! */
    task_t *old_task = (task_t*)current_task;
    current_task = next_task;
    
    /* Move CR3 if addressing spaces differ (identity mapping right now, but good practice) */
    if (old_task->cr3 != current_task->cr3) {
        __asm__ __volatile__("mov %0, %%cr3" : : "r"(current_task->cr3));
    }

    /* Set up magic EAX return value right before swapping stacks */
    __asm__ __volatile__("mov $0x12345678, %%eax" : : );
    
    /* Swap Stacks in ASM! */
    switch_task(&old_task->esp, current_task->esp);
}

void create_kernel_thread(void (*entry_point)(void)) {
    __asm__ __volatile__("cli");

    /* Allocate Task Control Block */
    task_t *new_task = (task_t *)memory_alloc(sizeof(task_t));
    new_task->id = next_pid++;
    new_task->cr3 = current_task->cr3; /* Shared exact same memory pool */

    /* Allocate a 4KB Stack for this unique thread */
    uint32_t *stack = (uint32_t *)memory_alloc(4096);
    uint32_t stack_top = (uint32_t)stack + 4096;

    /* Format the stack exactly how switch_task expects it to be laid out! */
    /* switch_task will pop EBP, EBX, ESI, EDI and then call 'ret' */
    
    uint32_t *sp = (uint32_t *)stack_top;
    
    *(--sp) = (uint32_t)entry_point; /* Return address (EIP) popped by 'ret' */
    *(--sp) = 0; /* EBP */
    *(--sp) = 0; /* EBX */
    *(--sp) = 0; /* ESI */
    *(--sp) = 0; /* EDI */

    new_task->esp = (uint32_t)sp;
    new_task->ebp = 0;
    new_task->eip = (uint32_t)entry_point;

    /* Inject this new task into the current Round Robin Linked List */
    new_task->next = ready_queue;
    
    /* Find the tail to link into the fresh circle */
    task_t *tail = (task_t *)ready_queue;
    while (tail->next != ready_queue) {
        tail = tail->next;
    }
    tail->next = new_task;

    __asm__ __volatile__("sti");
}
