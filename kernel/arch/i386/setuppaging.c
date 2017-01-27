#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <kernel/setuppaging.h>
#include <kernel/intex.h>

uint32_t kernelpagedir[1024] __attribute__ ((aligned (4096)));
uint32_t lowpagetable[1024] __attribute__ ((aligned (4096)));

/*
 * Initialize paging, and start by mapping the first 1MB and the kernel which exists from 1MB to a little past
 * 1MB into the page table. Then this page table is identity mapped to 0x00000000, and also mapped in the
 * higher half to 0xC0000000. Lastly, the page directory itself is mapped as a page table to the last 
 * page directory entry, so that at any time the page directory and the page tables can be accessed through
 * addresses at 0xFFF00000
 */
// Returns the number of pages mapped for the kernel
uint32_t get_pd_pa() {
    return (uint32_t)kernelpagedir;
}

void page_fault(regs_t *regs) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    unsigned int faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    // The error code gives us details of what happened.
    int present   = !(regs->err_code & 0x1); // Page not present
    int rw = regs->err_code & 0x2;           // Write operation?
    int us = regs->err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    printf("Page fault! (present: %d, read-only: %d, user-mode: %d, reserved: %d, ins_fetch: %d) at %x - EIP - %x\n", present>0, rw>0, us>0, reserved>0, id>0, faulting_address, regs->eip);

    for(;;);
}

uint32_t init_paging(uint32_t kernelstart, uint32_t kernelend) {
    void *kernelpagedirPtr = (char *)kernelpagedir + 0x40000000; // adding 4 is the same as subtracting C
    void *lowpagetablePtr = (char *)lowpagetable + 0x40000000;
 
    // First, map the page directory. All entries by default are 0, meaning that they don't exist
    for (int i = 0; i < 1024; i++) {
        kernelpagedir[i] = 0x00000000; // These will all be 0, meaning that most of memory is not mapped
    }

    kernelpagedir[0] = ((uint32_t)lowpagetablePtr) | 7; // ID map the kernel, just so it can keep executing
    kernelpagedir[768] = ((uint32_t)lowpagetablePtr) | 7; // Map to the higher half
    kernelpagedir[1023] = ((uint32_t)kernelpagedirPtr) | 7; // Map the page directory to itself as a table

    // Then map the first ~1MB of RAM, up through exactly the end of the kernel, into the first page table
    for (int k = 0; k < 1024; k++) {
        if (k <= kernelend/4096) { // I need one more page here to make space for the large memory manager bitmap
            lowpagetable[k] = (k*4096) | 7;
        } else {
            lowpagetable[k] = 0x00000000;
        }
    }
  
    asm volatile (  "mov %0, %%eax\n"
            "mov %%eax, %%cr3\n"
            "mov %%cr0, %%eax\n"
            "orl $0x80000000, %%eax\n"
            "mov %%eax, %%cr0\n" :: "m" (kernelpagedirPtr));

    irq_install_handler(14, &page_fault);
    return kernelend/4096+1; // The +1 is here because of the <=
}

void unmap_idmap() {
    uint32_t* kernelpagedir_real = (uint32_t*)(0xFFFFF000);
    kernelpagedir_real[0] = 0x00000000;
}

