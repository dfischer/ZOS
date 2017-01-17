#include <stdint.h>
#include <stdio.h>

#include <kernel/pmmngr.h>

uint32_t* get_pagedir() {
    return (uint32_t*)0xFFFFF000;
}

uint32_t* get_pagetable(uint32_t i) {
    return (uint32_t*)(0xFFC00000+i*0x1000);
}

// In general there will be issues if we want to deal with swapping to disk
// but for now, just check that an entry is 0 will be enough
void* allocate_page() {
    uint32_t physical_address = pmmngr_alloc_block();

    if (!physical_address) return (void*)0;

     // For now we should map this physical address to somewhere past 0xC0000000, so that for whatever reason
     // if the kernel needed to be accessed from user space it could see the whole kernel, not just
     // the statically allocated part of it... I don't know if that's something you would want, but 
     // it seems right
     
    // First see if there is room in any existing page tables
    uint32_t* pd = get_pagedir();
    for (int i = 768; i < 1023; i++){ // This means we're only allocating pages past 0xC0000000
        if (pd[i] & 1) {
            uint32_t* pt = get_pagetable(i);
            for (int j = 0; j < 1024; j++) {
                if (!(pt[j] & 1)) { // If the PTE does not exist, then we're good!
                    pt[j] = physical_address | 3;
                    void* newpage = (void*)(i*0x400000+j*0x1000);
                    memset(newpage, 0x00, 0x1000);
                    return newpage;
                }
            }
        }
    }

    // Next see if we can make a new page table
    
    for (int i = 768; i < 1023; i++) {
        if (!(pd[i] & 1)) {
            uint32_t page_table_paddress = pmmngr_alloc_block();
            printf("physical address of new page: %x\n", physical_address);
            printf("physical address of new page table | 3: %x\n", page_table_paddress | 3);
            if (!page_table_paddress) return (void*)0;
            pd[i] = page_table_paddress | 3;
            uint32_t* pt = get_pagetable(i);
            pt[0] = physical_address | 3;
            void* newpage = (void*)(i*0x400000);
            memset(newpage, 0x00, 0x1000);
            return newpage;
        }
    }

    return (void*)0;
}

// Both deallocate the physical page, and unmap the virtual page. The unmapping probably isn't really necessary,
// but it's good practice and it will be needed when we start talking about userspace things. In this vein,
// if the page table is now empty we will also deallocate and unmap the empty page table
void free_page(void* vaddr_ptr) {
    uint32_t vaddr = (uint32_t) vaddr_ptr;

    int pde_i = vaddr / 0x400000; // Each page table maps 4 MB = 0x400000
    int pte_i = (vaddr % 0x400000) / 0x1000;

    pmmngr_free_block(get_pagetable(pde_i)[pte_i]);

    // First, unmap the memory location
    get_pagetable(pde_i)[pte_i] = 0x00000000;
    asm volatile (  "mov %%cr3, %%ecx\n"
              "mov %%ecx, %%cr3\n" :); //Force the changes to take effect

    // Then see if we have to unmap the page table
    for (int i = 0; i < 1024; i++) {
        if (get_pagetable(pde_i)[i]) return;
    }

    // If we've got this far, then the table didn't have any entries so we can free the memory and unmap the pt
    pmmngr_free_block(get_pagedir()[pde_i]);

    get_pagedir()[pde_i] = 0x00000000;
    asm volatile (  "mov %%cr3, %%ecx\n"
              "mov %%ecx, %%cr3\n" :); //Force the changes to take effect
}