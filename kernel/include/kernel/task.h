#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>
#include <kernel/intex.h>

uint32_t initial_esp; // This will have the initial location of the start of the stack (highest memory address)

typedef struct task {
    int id;                // Process ID.
    uint32_t esp;       // Stack and base pointers.
    //uint32_t eip;            // Instruction pointer.
    uint32_t page_directory; // physical address of the page directory. This will be in the kernel heap, so it should be able to be accessed from anywhere
    struct task *next;     // The next task in a linked list.
} task_t;

int getpid();
int kfork();
void switch_task();
void switch_to_user_mode();
void init_multitasking(uint32_t initial_pd_physical_addr);

#endif
