#include <stdint.h>
#include <stddef.h>

#include <kernel/setuppaging.h>

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
uint32_t init_paging(uint32_t kernelstart, uint32_t kernelend) {
    void *kernelpagedirPtr = (char *)kernelpagedir + 0x40000000; // adding 4 is the same as subtracting C
    void *lowpagetablePtr = (char *)lowpagetable + 0x40000000;
 
    // First, map the page directory. All entries by default are 0, meaning that they don't exist
    for (int i = 0; i < 1024; i++) {
        kernelpagedir[i] = 0x00000000; // These will all be 0, meaning that most of memory is not mapped
    }

    kernelpagedir[0] = ((uint32_t)lowpagetablePtr) | 3; // ID map the kernel, just so it can keep executing
    kernelpagedir[768] = ((uint32_t)lowpagetablePtr) | 3; // Map to the higher half
    kernelpagedir[1023] = ((uint32_t)kernelpagedirPtr) | 3; // Map the page directory to itself as a table

    // Then map the first ~1MB of RAM, up through exactly the end of the kernel, into the first page table
    for (int k = 0; k < 1024; k++) {
        if (k <= kernelend/4096) { // I need one more page here to make space for the large memory manager bitmap
            lowpagetable[k] = (k*4096) | 3;
        } else {
            lowpagetable[k] = 0x00000000;
        }
    }
  
    asm volatile (  "mov %0, %%eax\n"
            "mov %%eax, %%cr3\n"
            "mov %%cr0, %%eax\n"
            "orl $0x80000000, %%eax\n"
            "mov %%eax, %%cr0\n" :: "m" (kernelpagedirPtr));

    return kernelend/4096+1; // The +1 is here because of the <=
}

void unmap_idmap() {
    uint32_t* kernelpagedir_real = (uint32_t*)(0xFFFFF000);
    kernelpagedir_real[0] = 0x00000000;
}