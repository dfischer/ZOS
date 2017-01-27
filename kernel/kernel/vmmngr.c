#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel/pmmngr.h>
#include <kernel/kmalloc.h>

uint32_t* get_pagedir() {
    return (uint32_t*)0xFFFFF000;
}

uint32_t* get_pagetable(uint32_t i) {
    return (uint32_t*)(0xFFC00000+i*0x1000);
}

void* allocate_pages(int np) {
    if (np > 4000) {
        printf("Error, unable to allocate %d pages in sequence. The maximum is 4000\n", np);
        return 0;
    }

    uint32_t* paddrs = kmalloc(sizeof(uint32_t)*np);
    for (int i = 0; i < np; i++) {
        paddrs[i] = pmmngr_alloc_block();
        if (!paddrs[i]) {
            for (int j = 0; j < i; j++) {
                pmmngr_free_block(paddrs[j]);
            }
            printf("Error, unable to allocate a physical page in allocate_pages. Aborting\n");
            return 0;
        }
    }

    // First see if there is room in any existing page tables
    uint32_t* pd = get_pagedir();
    for (int i = 768; i < 896; i++){ // This means we're only allocating pages past 0xC0000000
        if (pd[i] & 1) {
            uint32_t* pt = get_pagetable(i);

            uint32_t start_idx = 0;
            for (int j = 0; j < 1024; j++) {
                if (pt[j] & 1) { // If the PTE exists, move on
                    start_idx = j+1;
                } else {
                    uint32_t num_avail = j-start_idx+1;
                    if (num_avail >= np) {
                        for (int k = start_idx; k <= j; k++) {
                            pt[k] = paddrs[k-start_idx] | 7;
                        }
                        void* newpage = (void*)(i*0x400000+start_idx*0x1000);
                        memset(newpage, 0x00, 0x1000*np);
                        return newpage;
                    }
                }
            }
        }
    }

    // Next see if we can make a new page table
    
    for (int i = 768; i < 896; i++) {
        if (!(pd[i] & 1)) {
            uint32_t page_table_paddress = pmmngr_alloc_block();
            if (!page_table_paddress) {
                printf("Error, unable to allocate a physical page for the new page table in allocate_pages\n");
                return (void*)0;
            }
            pd[i] = page_table_paddress | 7;
            uint32_t* pt = get_pagetable(i);
            memset(pt, 0x00, 0x1000); // We need to first clear the page table

            for (int k = 0; k < np; k++) {
                pt[k] = paddrs[k] | 7;
            }
            void* newpage = (void*)(i*0x400000);
            memset(newpage, 0x00, 0x1000*np);
            return newpage;
        }
    }

    printf("Error, unable to allocate a page, not enough space on the heap. Consider making the heap resizable\n");
    for (int i = 0; i < np; i++) {
        pmmngr_free_block(paddrs[i]);
    }
    return (void*)0;
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
    for (int i = 768; i < 896; i++){ // This means we're only allocating pages past 0xC0000000
        if (pd[i] & 1) {
            uint32_t* pt = get_pagetable(i);
            for (int j = 0; j < 1024; j++) {
                if (!(pt[j] & 1)) { // If the PTE does not exist, then we're good!
                    pt[j] = physical_address | 7;
                    void* newpage = (void*)(i*0x400000+j*0x1000);
                    memset(newpage, 0x00, 0x1000);
                    return newpage;
                }
            }
        }
    }

    // Next see if we can make a new page table
    
    for (int i = 768; i < 896; i++) {
        if (!(pd[i] & 1)) {
            uint32_t page_table_paddress = pmmngr_alloc_block();
            printf("physical address of new page: %x\n", physical_address);
            printf("physical address of new page table | 3: %x\n", page_table_paddress | 3);
            if (!page_table_paddress) return (void*)0;
            pd[i] = page_table_paddress | 7;
            uint32_t* pt = get_pagetable(i);
            memset(pt, 0x00, 0x1000); // We need to first clear the page table
            pt[0] = physical_address | 3;
            void* newpage = (void*)(i*0x400000);
            memset(newpage, 0x00, 0x1000);
            return newpage;
        }
    }

    printf("Error, unable to allocate a page, not enough space on the heap. Consider making the heap resizable\n");
    pmmngr_free_block(physical_address);
    return (void*)0;
}

// I am 95% sure this is correct, even though parts seem weird. get_pagetable() gets a pointer to something like
// 0xFF000000, but you should still be able to set the entries, so setting them all to 0 should work fine, for
// example.
void* allocate_specific_page(uint32_t vaddr) {
    int pde_i = vaddr / 0x400000; // Each page table maps 4 MB = 0x400000
    int pte_i = (vaddr % 0x400000) / 0x1000;

    if (get_pagedir()[pde_i]) {
        if (get_pagetable(pde_i)[pte_i]) {
            printf("Error allocating page %x, this page has already been allocated\n", vaddr);
            return 0;
        }
    } else {
        get_pagedir()[pde_i] = pmmngr_alloc_block() | 7;
        uint32_t* new_pt = get_pagetable(pde_i);
        memset(new_pt, 0x00, 0x1000);
    }

    get_pagetable(pde_i)[pte_i] = pmmngr_alloc_block() | 7;
    void* newpage = (void*) vaddr;
    memset(newpage, 0x00, 0x1000);

    return newpage;
}

void allocate_user_pages(uint32_t vaddr, uint32_t size) {
    int num_pages = size/0x1000;

}

// Both deallocate the physical page, and unmap the virtual page. The unmapping probably isn't really necessary,
// but it's good practice and it will be needed when we start talking about userspace things. In this vein,
// if the page table is now empty we will also deallocate and unmap the empty page table
void free_page(void* vaddr_ptr) {
    uint32_t vaddr = (uint32_t) vaddr_ptr;

    int pde_i = vaddr / 0x400000; // Each page table maps 4 MB = 0x400000
    int pte_i = (vaddr % 0x400000) / 0x1000;

    pmmngr_free_block(get_pagetable(pde_i)[pte_i] & 0xFFFFF000);

    // First, unmap the memory location
    get_pagetable(pde_i)[pte_i] = 0x00000000;
    asm volatile (  "mov %%cr3, %%ecx\n"
              "mov %%ecx, %%cr3\n" :); //Force the changes to take effect

    // Then see if we have to unmap the page table
    for (int i = 0; i < 1024; i++) {
        if (get_pagetable(pde_i)[i]) return;
    }

    // If we've got this far, then the table didn't have any entries so we can free the memory and unmap the pt
    pmmngr_free_block(get_pagedir()[pde_i] & 0xFFFFF000);

    get_pagedir()[pde_i] = 0x00000000;
    asm volatile (  "mov %%cr3, %%ecx\n"
              "mov %%ecx, %%cr3\n" :); //Force the changes to take effect
}

void free_pages(void* start, int np) {
    char* page = start;
    for (int i = 0; i < np; i++) {
        free_page(page);
        page += 0x1000;
    }
}

uint32_t get_physical_addr(void* vaddr_ptr) {
    uint32_t vaddr = (uint32_t) vaddr_ptr;
  
    int pde_i = vaddr / 0x400000; // Each page table maps 4 MB = 0x400000
    int pte_i = (vaddr % 0x400000) / 0x1000;
    uint32_t offset = vaddr % 0x1000;

    if (get_pagedir()[pde_i]) {
        if (get_pagetable(pde_i)[pte_i]) {
            return (get_pagetable(pde_i)[pte_i]&0xFFFFF000)+offset;
        } else {
            printf("Unable to find a page table entry for virtual address %x\n", (uint32_t)vaddr_ptr);
            return 0;
        }
    } else {
        printf("Unable to find a page directory entry for virtual address %x\n", (uint32_t)vaddr_ptr);
        return 0;
    }
}

// This will clone the current page directory, and any tables that have been allocated, and return the physical address of 
// the new page directory.
uint32_t clone_current_pd() {
    uint32_t* new_pd = allocate_page();
    uint32_t* current_pd = get_pagedir();
    
    for (int i = 0; i < 1023; i++) {
        if (i >= 768 && i < 896) { // Between 0xC0000000 and 0xE0000000, so it should be linked
            new_pd[i] = current_pd[i];
        } else if(current_pd[i]) { // If a table outside the range exists, it should be copied
            uint32_t* new_pt = allocate_page();
            uint32_t* current_pt = get_pagetable(i);
            unsigned short pde_flags = current_pd[i] & 0xFFF;

            for (int j = 0; j < 1024; j++) {
                if (!current_pt[j]) continue;
                unsigned char* data = allocate_page();
                memcpy(data, (void*)(i*0x400000+j*0x1000), 0x1000);
                unsigned short pte_flags = current_pt[j] & 0xFFF;
                new_pt[j] = get_physical_addr(data) | pte_flags; // We need to set it with the physical address
            }
            new_pd[i] = get_physical_addr(new_pt) | pde_flags;
        }
    }

    // Finish by ID mapping the new page directory
    unsigned short pd_flags = current_pd[1023] & 0xFFF;
    new_pd[1023] = get_physical_addr(new_pd) | pd_flags;

    return get_physical_addr(new_pd); // We are returning the physical address of the page directory
}

// This gets the virtual address of a physical address that is stored in the heap
// Note that this will only work if the physical address is stored in the heap!
void* get_virtual_addr(uint32_t physical_addr) {
    uint32_t start_page = physical_addr & 0xFFFFF000;
    uint32_t offset = physical_addr & 0xFFF;
    
    uint32_t* pd = get_pagedir();

    // The heap is only part of virtual memory
    for (int i = 768; i < 896; i++) {
        if (!pd[i]) continue;
        uint32_t* pt = get_pagetable(i);
        for (int j = 0; j < 1024; j++) {
            if ((pt[j]&0xFFFFF000) == start_page) {
                return (void*)(i*0x400000+j*0x1000+offset);
            }
        }
    }

    printf("Error, unable to find physical address %x in the heap\n", physical_addr);
    return 0;
}

void free_pd(uint32_t pd_physical) {
    uint32_t* pd = get_virtual_addr(pd_physical); // The actual page directory

    if (!pd) {
        printf("Are you trying to free the page directory of the initial process? This cannot be done because the page directory is not stored in the heap, it's stored in the kernel code. Physical pd address: %x\n", pd_physical);
        return;
    }

    for (int i = 0; i < 1023; i++) {
        if (i >= 768 && i < 896) continue;
        if (!pd[i]) continue;
        uint32_t* pt = get_virtual_addr(pd[i]&0xFFFFF000); // The virtual address of the page table
        for (int j = 0; j < 1024; j++) {
            if (!pt[j]) continue;
            unsigned char* data = get_virtual_addr(pt[j]&0xFFFFF000); // The virtual address of the page itself
            free_page(data);
        }
        free_page(pt);
    }
    free_page(pd);
}
