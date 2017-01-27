#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <kernel/multiboot.h>
#include <kernel/tty.h>
#include <kernel/setuppaging.h>
#include <kernel/gdt.h>
#include <kernel/pmmngr.h>
#include <kernel/vmmngr.h>
#include <kernel/kmalloc.h>
#include <kernel/intex.h>
#include <kernel/kb.h>
#include <kernel/timer.h>
#include <drivers/ata.h>
#include <drivers/fat32.h>
#include <kernel/terminal.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/read_elf.h>
#include <kernel/syscall.h>
 
extern uint32_t kernelStart;
extern uint32_t kernelEnd;

int testp(int p1) {
    printf("this is a test\n");

    for (int i = 0; i < 10; i++) {
        p1 += 2*i;
    }

    return p1;
}

void kernel_main(multiboot_info_t *mbd, uint32_t initial_stack) {
    initial_esp = initial_stack;
    uint32_t pd_physical_addr = get_pd_pa();
    uint32_t kernelpages = init_paging((uint32_t)&kernelStart, (uint32_t)&kernelEnd);
    gdt_install();
    
    idt_install();
    //isrs_install(); // Taken out to streamline everything in irq.c
    irq_install();

    terminal_initialize();
    
    pmmngr_init(mbd, kernelpages);
    printf("kernelStart: %x\n", (uint32_t)&kernelStart);
    printf("kernelEnd: %x\n", (uint32_t)&kernelEnd);
    //printf("mem_lower (bytes): %x\n", mbd->mem_lower*1024);
    //printf("mem_upper (bytes): %x\n", mbd->mem_upper*1024);
    unmap_idmap(); // For now we'll do this here, beacuse mbd uses physical addresses, so we need the ID map still
    
    kmalloc_init();

    kb_install();
    timer_install();

    __asm__ __volatile__ ("sti");

    ide_initialize(0x1F0, 0x3F4, 0x170, 0x374, 0x000);

    for (int i = 1020; i < 1024; i++) {
        printf("%x ", get_pagedir()[i]);
    }
    printf("\n");
    printf("%x\n", (uint32_t)&initial_stack);

    unsigned int pnum = 1; // Just for now, assume that we want to use hdd and partition 5
    
    init_fat32();

    

    printf("mounting drive...\n");
    mount(hdd, pnum, 'a');
    /*int fd = open("a:/st_d_4095.txt");
    unsigned char* buffer = kmalloc(get_block_size(fd));
    for (int i = 0; i < 8; i++) {
        int res = read(fd, buffer);

        int num_0 = 0;
        for (int j = 0; j < get_block_size(fd); j++) {
            if (buffer[j] == 0) num_0++;
        }

        printf("read res: %x, num_0: %d\n", res, num_0);
    }
    kfree(buffer);
    close(fd);*/

    /*
    for (int i = 0; i < 10; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n\n");
    for (int i = 760; i < 780; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");*/
    //printf("opening file...\n");

    //uint32_t old_stack_pointer; __asm__ __volatile__("mov %%esp, %0" : "=r" (old_stack_pointer));
    //printf("old stack pointer: %x\n", old_stack_pointer);

    //printf("result of get_pagedir(): %x. Result of get_pagetable(768): %x\n", get_pagedir(), get_pagetable(768));

    init_multitasking(pd_physical_addr+0x40000000);

    init_syscalls();

    /*int pid = kfork();

    if (pid) { // This is the parent
        
        uint32_t kernel_stack;
    }*/



    /*uint32_t kernel_stack;
    __asm__ __volatile__("mov %%esp, %0" : "=r"(kernel_stack));
    set_kernel_stack(kernel_stack-8); // This -8 is a huge hack. I need to think more carefully about where exactly the stack is set from. Also it will be a different case when the kernel and the user mode are not using the same stack

    switch_to_user_mode();

    char t = 't';
    syscall_terminal_write(&t, 1);*/







    int res = read_elf("a:/test");
    //read_elf("a:/test_old");


    /*for (int i = 760; i < 780; i++) {
        printf("%x ", get_pagedir()[i]);
    }
    printf("\n");


    for (int i = 890; i < 910; i++) {
        printf("%x ", get_pagedir()[i]);
    }
    printf("\n");

    printf("%x\n", get_pagedir()[1023]);

    for (int i = 0; i <20; i++) {
        printf("%x ", get_pagetable(768)[i]);
    }
    printf("\n");

    for (int i = 0; i <20; i++) { 
        printf("%x ", get_pagetable(896)[i]);
    } 
    printf("\n");*/

    /*int ret = fork();

    printf("fork() returned %x, getpid() returned %x\n", ret, getpid());

    for (int i = 1; i < 10; i++) {
        int tmp = 73;
        for (int j = 1; j < 2872389; j++) {
            tmp += ((117234253*i*j) % 5);
        }
        printf("%d ", tmp);
    }*/

    //__asm__ __volatile__("mov %%esp, %0" : "=r" (old_stack_pointer));
    //printf("new stack pointer: %x\n", old_stack_pointer);

    /*FILE* file = fopen("a:/this is a file.txt", "r");
    //int res = fgetc(file);

    for (int i = 0; i < 100; i++) {
        printf("%x", fgetc(file));
    }

    int res2 = fclose(file);*/
    /*
    int fd = open("a:/this is a file.txt");

    unsigned char* buffer = allocate_page();
    int res2 = read(fd, buffer);
    printf("\n%s\n\n", buffer);

    printf("closing file with fd %d...\n", fd);
    int res = close(fd);
    printf("result: %d\n", res);
    */

    //init_terminal(rootnode);

    //const char* diskID = kmalloc(11);
    //memcpy(diskID, (unsigned char*) (ptr+0x1b4), 10);
    //printf("disk ID: %s", diskID);
    //kfree(diskID);

    /*printf("First 100 shorts = 200 bytes of the disk:\n");
    for (int i = 0; i < 100; i++) {
        printf("%x ", buffer[i]);
    }*/

    //uint32_t newaddr = pmmngr_alloc_block();
    //printf("new address: %x\n", newaddr);

    //print_map(0, 20);

    /*
    uint32_t* newpage = (uint32_t*)allocate_page();
    printf("newpage virtual address: %x, value: %x\n", newpage, *newpage);

    print_map(0, 20);

    uint32_t vaddr = (uint32_t) newpage;
    int pde_i = vaddr / 0x400000; // Each page table maps 4 MB = 0x400000
    int pte_i = (vaddr % 0x400000) / 0x1000;

    printf("new page directory index: %d, table index: %d\n", pde_i, pte_i);
    printf("new page physical address: %x\n", get_pagetable(pde_i)[pte_i]);
    *newpage = 0xDEADC0DE;
    free_page(newpage);

    print_map(0, 20);
    printf("newpage virtual address: %x, value: %x\n", newpage, *newpage);
    printf("page directory entry: %x\n", get_pagedir()[pde_i]);
    printf("page table entry: %x\n", get_pagetable(pde_i)[pte_i]);
    */

    /*uint32_t* newpage;
    for (int i = 0; i < 0x2D9; i++) {
        newpage = (uint32_t*)allocate_page();
    }

    printf("newpage virtual address: %x, value: %x\n", newpage, *newpage);   

    uint32_t* pd = get_pagedir();
    printf("pagedir[769]: %x\n\n", (uint32_t)(pd[769]));

    free_page(newpage);
    printf("pagedir[769]: %x\n\n", (uint32_t)(pd[769]));*/


    //print_map(0, 20);

    //NEXT UP, SET UP A SYSTEM TO MAP THE NEW PAGE INTO THE VIRTUAL ADDRESS SPACE, AND THINK ABOUT REORGANIZING MAYBE
    printf("Done.");
    syscall_hlt();
    //__asm__ __volatile__("int $0x80");
}
