#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/vmmngr.h>
#include <kernel/task.h>
#include <kernel/kmalloc.h>
#include <kernel/elf.h>
#include <kernel/gdt.h>

#include <kernel/pmmngr.h> // This can be taken out. just for debugging memory freeing

// The first one will be running, the rest will be waiting
task_t* task_queue;
uint32_t next_pid = 0;

extern uint32_t read_eip();

int getpid() {
    if (!task_queue) return -1;
    return task_queue->id;
}

// This will move the original stack to a different location. This will have to be changed to accomodate changing it further
// This really should just be a one time thing...
// new_stack points to the lowest address of the stack, i.e. the top of the stack
void move_stack(void* new_stack, uint32_t size) {
    uint32_t new_stack_start = (uint32_t)new_stack + size;

    // This will actually allocate 4095 bytes too much, but it's better to be safe than sorry with the kernel's stack...
    for (uint32_t i = new_stack_start; i >= (uint32_t)new_stack; i -= 0x1000) {
        allocate_specific_page(i, 1, 1); // writable, not user space
    }
    // We may need to change the page table, but I'm not sure...

    uint32_t old_stack_pointer; __asm__ __volatile__("mov %%esp, %0" : "=r" (old_stack_pointer));
    uint32_t old_base_pointer;  __asm__ __volatile__("mov %%ebp, %0" : "=r" (old_base_pointer));

    uint32_t offset = new_stack_start - initial_esp;
    uint32_t new_stack_ptr = old_stack_pointer + offset;
    uint32_t new_base_ptr = old_base_pointer + offset;

    // Copy over the stack. Only copy however much has been used
    memcpy((void*)new_stack_ptr, (void*)old_stack_pointer, initial_esp-old_stack_pointer);

    // Finally we need to find any place EBP (base pointer) has been pushed previously, and change the 
    // occurance to the correct new base pointer. As this can happen many times, we need to change all
    // of them correctly by adding the offset
    
    // Go through each byte and if the value in the new stack is in the range of the old stack, 
    // we'll assume it's a EBP. There may be some items that have a value that happens to be in the range
    // of the stack, and if so those values will be screwed up. This is unlikely, however.
    for (uint32_t i = new_stack_start; i >= (uint32_t)new_stack; i -= 4) { // this might have to be > instead of >=
        uint32_t stack_val_i = * (uint32_t*)i;
        if ((stack_val_i >= old_stack_pointer) && (stack_val_i <= initial_esp)) {
            // Then we'll just assume it's a EBP
            *(uint32_t*)i = stack_val_i + offset;
        }
    }

    // Now everything is in place, let's just change the stack pointer and base pointer:
    __asm__ __volatile__("mov %0, %%esp" : : "r" (new_stack_ptr));
    __asm__ __volatile__("mov %0, %%ebp" : : "r" (new_base_ptr));
}

uint32_t* lastpage;
int lastoffset;
static void fake_push(uint32_t* esp_ptr, uint32_t pd_pa, uint32_t val) {
    *esp_ptr  = (*esp_ptr)-4; // First we need to make room for the value

    uint32_t pde_i = (*esp_ptr) / 0x400000;
    uint32_t pte_i = ((*esp_ptr) % 0x400000) / 0x1000;
    int offset = (*esp_ptr) % 0x1000;

    // If offset > lastoffset, we have wrapped around to a new page
    if (offset > lastoffset) {
        uint32_t* pd = get_virtual_addr(pd_pa);
        uint32_t* pt = get_virtual_addr(pd[pde_i]&0xFFFFF000); // This is the new, copied page table
        uint32_t* page = get_virtual_addr(pt[pte_i]&0xFFFFF000);
        lastpage = page;
    }

    lastoffset = offset;

    uint32_t* esp =  (uint32_t*)((uint32_t)lastpage+offset);
    *esp = val; // Actually set the value
}

int extra_bytes = -1; // This is going to be a hack. We need to know how many extra uint32s we need to add to
                        // a new processes stack so returns through all the functions correctly
uint32_t* correct_return_codes = 0;

int kfork() {
    __asm__ __volatile__("cli"); // Disable interrupts

    task_t* parent_task = task_queue; // Remember the current running process originally
    uint32_t new_addr_space = clone_current_pd();

    task_t* new_task = kmalloc(sizeof(task_t));
    new_task->id = next_pid++;
    new_task->page_directory = new_addr_space;

    // This was some debugging stuff, trying to get clone_current_pd() to work, but now I think it's 
    // working well.
    /*uint32_t* pd = get_virtual_addr(new_addr_space);

    printf("current pd page directory and page table[768]:\n");

    for (int i = 890; i < 910; i++) {
        printf("%x ", get_pagedir()[i]);
    }
    printf("\n");

    for (int i = 0; i <20; i++) {
        printf("%x ", get_pagetable(896)[i]);
    }
    printf("\n");

    printf("new pd page directory and page table[768]:\n");

    for (int i = 890; i < 910; i++) { 
        printf("%x ", pd[i]);
    } 
    printf("\n");


    for (int i = 890; i < 910; i++) { 
        printf("%x ", pd[i]);
    } 
    printf("\n");

    printf("%x\n", pd[1023]);

    uint32_t* pt = get_virtual_addr(pd[896]&0xFFFFF000);

    for (int i = 0; i <20; i++) { 
        printf("%x ", pt[i]);
    } 
    printf("\n");

    pt = (uint32_t*)(pd[896]+0xC0000000);
    for (int i = 0; i <20; i++) {
        printf("%x ", pt[i]);
    }
    printf("\n");*/

    //new_task->esp = new_task->ebp = new_task->eip = 0;
    //new_task->next = 0;

    task_t* queue = task_queue;
    while (queue->next) {
        queue = queue->next;
    }
    queue->next = new_task;

    uint32_t eflags; __asm__ __volatile__("pushf; pop %0" : "=r"(eflags));
    uint32_t cs; __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));          
    uint32_t eax; __asm__ __volatile__("mov %%eax, %0" : "=r"(eax));       
    uint32_t ecx; __asm__ __volatile__("mov %%ecx, %0" : "=r"(ecx));       
    uint32_t edx; __asm__ __volatile__("mov %%edx, %0" : "=r"(edx));       
    uint32_t ebx; __asm__ __volatile__("mov %%ebx, %0" : "=r"(ebx));       
    uint32_t esp; __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));       
    uint32_t ebp; __asm__ __volatile__("mov %%ebp, %0" : "=r"(ebp));       
    uint32_t esi; __asm__ __volatile__("mov %%esi, %0" : "=r"(esi));       
    uint32_t edi; __asm__ __volatile__("mov %%edi, %0" : "=r"(edi));       
    uint32_t ds; __asm__ __volatile__("mov %%ds, %0" : "=r"(ds));          

    uint32_t eip = read_eip();
    // This will be the entry point, because it's here that we're saving the eip. So here we could be either the parent or the child
    
    if (task_queue == parent_task) {
        // We are the parent, so set up the registers for our child.

        lastpage = 0;
        lastoffset = -1;

        uint32_t esp_ptr; __asm__ __volatile__("mov %%esp, %0" : "=r"(esp_ptr));

        fake_push(&esp_ptr, new_addr_space, eflags);
        fake_push(&esp_ptr, new_addr_space, cs);
        fake_push(&esp_ptr, new_addr_space, eip);
        fake_push(&esp_ptr, new_addr_space, 0);
        fake_push(&esp_ptr, new_addr_space, 0);
        fake_push(&esp_ptr, new_addr_space, eax);
        fake_push(&esp_ptr, new_addr_space, ecx);
        fake_push(&esp_ptr, new_addr_space, edx);
        fake_push(&esp_ptr, new_addr_space, ebx);
        fake_push(&esp_ptr, new_addr_space, esp);
        fake_push(&esp_ptr, new_addr_space, ebp);
        fake_push(&esp_ptr, new_addr_space, esi);
        fake_push(&esp_ptr, new_addr_space, edi);
        fake_push(&esp_ptr, new_addr_space, ds);
        //fake_push(&esp_ptr, new_addr_space, new_addr_space);
        for (int i = extra_bytes/4-1; i >=0; i--) {
            fake_push(&esp_ptr, new_addr_space, correct_return_codes[i]);
        }

        new_task->esp = esp_ptr; // Now we pretend the stack pointer has a bunch of random stuff
                                               // from other functions

        // All finished: Reenable interrupts.
        __asm__ __volatile__("sti");
        return new_task->id;
    } else {
        __asm__ __volatile__("sti"); // We need this because when the program resumes the flags are taken from before, i.e. from when interrupts were disabled
        return 0; // By convention. We don't need to re-enable interrupts because the parent has already done it!
    }
}

char* open_strtab(FILE* file, elf_header* header) {
    sheader_entry* strtab_header = kmalloc(sizeof(sheader_entry));
    int strtab_filepos = header->section_header_table_pos+header->section_name_pos*header->s_header_table_entry_size;
    printf("strtab position: %x\n", strtab_filepos);
    fseek(file, header->section_header_table_pos+header->section_name_pos*header->s_header_table_entry_size, SEEK_SET);
    fread(strtab_header, sizeof(sheader_entry), 1, file);

    char* string_table = kmalloc(strtab_header->size+1);
    fseek(file, strtab_header->offset, SEEK_SET);
    fread(string_table, strtab_header->size, 1, file);

    kfree(strtab_header);
    return string_table;
}

char* read_strtab(char* strtab, int idx) {
    return strtab+idx;
}

void close_strtab(char* string_table) {
    kfree(string_table);
}

extern void switch_to_user_mode(uint32_t entry, uint32_t user_stack);

int kexecv_elf(const char* filename, int argc, char* argv[], int stack_size, int heap_size) {
    FILE* file = fopen(filename, "r");

    if (!file) {
        printf("Error, unable to open file %s\n", filename);
        return 1;
    }

    elf_header* header = (elf_header*) kmalloc(sizeof(elf_header));
    fread(header, sizeof(elf_header), 1, file);

    if (header->magic != 0x464C457F) {
        printf("Error, the file is not an ELF executable, the magic bits are: %x\n", header->magic);
        return 2;
    }

    // Remove anything in userspace before replacing it with the new program
    clean_current_userspace();

    //printf("magic: %x, type: %x, entry_pos: %x, header table pos: %x, section table pos: %x header size: %x, header table entries: %x,   section table entries: %x, section_name_pos: %x, ptable_esize: %x, stable_esize: %x\n", header->magic, header->exe_type, header->pentry  _pos, header->program_header_table_pos, header->section_header_table_pos, header->header_size, header->p_header_table_num_entries, heade  r->s_header_table_num_entries, header->section_name_pos, header->p_header_table_entry_size, header->s_header_table_entry_size);
    uint32_t chunk_size = 512;

    pheader_entry* pheader_table = (pheader_entry*) kmalloc(sizeof(pheader_entry)*header->p_header_table_num_entries);


    fseek(file, header->program_header_table_pos, SEEK_SET);
    fread(pheader_table, sizeof(pheader_entry), header->p_header_table_num_entries, file);

    uint32_t last_vaddr = 0;
    for (int i = 0; i < header->p_header_table_num_entries; i++) {
        pheader_entry* ent = pheader_table+i;
            //printf("type: %x, offset: %x, vaddr: %x, size_file: %x, size_mem: %x, flags: %x, align: %x\n", ent->type, ent->data_offset, ent->vaddr,  ent->segment_size_file,  ent->segment_size_memory,  ent->flags,  ent->alignment);

        if (ent->type == 0x01) { // load
            int num_pages = ent->segment_size_memory/0x1000+1;
            size_t start_addr = ent->vaddr & 0xFFFFF000;
            for (int i = 0; i < num_pages; i++) {
                allocate_specific_page(start_addr, 1, 1); // For now... though later we should probably be more careful about permission  s
                start_addr += 0x1000;
            }

            memset((void*)ent->vaddr, 0, ent->segment_size_memory);
            uint32_t num_chunks = ent->segment_size_file/chunk_size+1;
            unsigned char* data = kmalloc(num_chunks*chunk_size);

            fseek(file, ent->data_offset, SEEK_SET);
            fread(data, chunk_size, num_chunks, file);
            memcpy((void*)ent->vaddr, data, ent->segment_size_file);

            kfree(data);
            
            uint32_t end_vaddr = ent->vaddr + ent->segment_size_memory;
            if (end_vaddr > last_vaddr) last_vaddr = end_vaddr;
        } else {
            printf("error, unable to handle program header table entry: type: %x, offset: %x, vaddr: %x, size_file: %x, size_mem: %x, fl  ags: %x, align: %x\n", ent->type, ent->data_offset, ent->vaddr,  ent->segment_size_file,  ent->segment_size_memory,  ent->flags,  ent->alignment);
        }
    }
    
    //print_map(8, 20);
    //printf("vaddr: %x\n", get_virtual_addr(1298432));
    kfree(pheader_table);


    // Not needed, because this is the wrong way to set the stack.
    /*char* string_table = open_strtab(file, header);

    sheader_entry* sheader_table = (sheader_entry*) kmalloc(sizeof(sheader_entry)*header->s_header_table_num_entries);

    fseek(file, header->section_header_table_pos, SEEK_SET);
    fread(sheader_table, sizeof(sheader_entry), header->s_header_table_num_entries, file);

    uint32_t newstack_ptr = 0;
    for (int i = 0x00; i < header->s_header_table_num_entries; i++) {
        sheader_entry* ent = sheader_table+i;
        if (strcmp(read_strtab(string_table, ent->name_offset), ".bss")  == 0) {
            newstack_ptr = ent->vaddr+ent->size;
        }
        //printf("name: %s, type: %x, attr: %x, vaddr: %x, offset: %x, size: %x, info: %x\n", read_strtab(string_table, ent->name_offset), ent->type, ent->offset,  ent->vaddr,  ent->offset,  ent->size,  ent->info);
    }

    close_strtab(string_table);
    kfree(sheader_table);*/

    // Allocate the stack
    uint32_t stack_start = ((last_vaddr-1) & 0xFFFFF000) + 0x1000;
    uint32_t stack_end = stack_start + ((stack_size-1) & 0xFFFFF000) + 0x1000;
    for (uint32_t i = stack_start; i < stack_end; i += 0x1000) {
        allocate_specific_page(i, 1, 1);
    }

    // Allocate the heap
    uint32_t heap_start = stack_end;
    uint32_t heap_end = heap_start + ((heap_size-1) & 0xFFFFF000) + 0x1000;
    for (uint32_t i = heap_start; i < heap_end; i += 0x1000) {
        allocate_specific_page(i, 1, 1);
    }


    uint32_t entry_pos = header->pentry_pos;
    kfree(header);
    fclose(file);

    task_queue->user_stack_bottom = stack_start;
    task_queue->user_stack_size = ((stack_size-1) & 0xFFFFF000) + 0x1000;
    task_queue->user_heap_ptr = heap_start;
    task_queue->user_heap_size = ((heap_size-1) & 0xFFFFF000) + 0x1000;

    // Set up the beginning of the stack to have argc and argv
    uint32_t* user_esp = (uint32_t*) (stack_end-4);
    *user_esp = (uint32_t)argv;
    user_esp--;
    *user_esp = (uint32_t)argc;
    stack_end = (uint32_t)user_esp;

    uint32_t kernel_stack;
     __asm__ __volatile__("mov %%esp, %0" : "=r"(kernel_stack));
    kernel_stack -= 0x40; // Set the kernel stack a little farther down, just in case

    task_queue->kernel_esp = kernel_stack;
    set_kernel_stack(kernel_stack);

    // This is an assembly function that swicthes to user mode with the correct eip and stack pointer
    switch_to_user_mode(entry_pos, stack_end);
    /*__asm__ __volatile__("\
            mov %3, %%esp;\
            push %1;\
            push %0;\
            jmp *%2" : : "r" (argc), "r" (argv),  "r" (entry_pos), "r" (stack_end));*/

    return 0; // This should never happen...
}

void start_process(const char* filename, int argc, char* argv[], int stack_size, int heap_size) {
    int pid = kfork();
    if (pid == 0) { // If we're the child
        kexecv_elf(filename, argc, argv, stack_size, heap_size);
        printf("Error, returned from kexecv_elf, this should never happen!\n");
        kill_current_process(1);
    }
}

void kill_current_process(int status) {
    //printf("killing process %d with status %d\n", task_queue->id, status);
    switch_task((void*)0, 1);
}

// This kills the current process, and runs the next one in the queue
// For this to happen, there must be at least two tasks running currently
/*void kill_current_process(int free_memory) {
    if (!task_queue && !task_queue->next) {
        return;
    }

    __asm__ __volatile__("cli"); // Disable interrupts, because we're dealing with switching tasks
    // Swap the active task
    task_t* current_task = task_queue;
    task_queue = current_task->next;
    kfree(current_task);
    if (free_memory)
        pd_paddr_to_free = current_task->page_directory;

    // Now all we have to do is put in the correct stack and address space, and the program will continue on
    // to return from the interrupt

    __asm__ __volatile__("mov %0, %%esp;" :: "r"(task_queue->esp));
    __asm__ __volatile__("mov %0, %%cr3;" :: "r"(task_queue->page_directory));
    __asm__ __volatile__("sti"); // Re-enable interrupts :)
}*/

/*void switch_out_of_kernel() {
    if (!task_queue && !task_queue->next) {
        return;
    }

    // Swap the active task
    
    task_t* kernel = task_queue;
    task_queue = kernel->next;
    kfree(kernel);

    // Now all we have to do is put in the correct stack and address space, and the program will continue on
    // to return from the interrupt

    __asm__ __volatile__("mov %0, %%esp;" :: "r"(task_queue->esp));
    __asm__ __volatile__("mov %0, %%cr3;" :: "r"(task_queue->page_directory));
}*/

// This will effectively initially be called by the task that is currently running
// We will need to save the state of that task, and then load the esp, ebp, and eip of the task to switch to
// We may have to worry about other exceptions happening in this handler...
void switch_task(regs_t* r, int kill) {
    __asm__ __volatile__("cli"); // Disable interrupts, because we're dealing with task switching
    // They will be reenabled upon return because the flags will be properly set from before the interrupt

    uint32_t esp; __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));
    if (!correct_return_codes) {
        extra_bytes = (uint32_t)r - esp;
        correct_return_codes = kmalloc(extra_bytes);
        memcpy(correct_return_codes, (uint32_t*)esp, extra_bytes);
    }

    // if there are 0 or 1 tasks in the task queue, we don't need to do anything
    if (!task_queue || !task_queue->next) {
        return;
    }

    if (kill) {
        task_t* current_task = task_queue;
        task_queue = current_task->next;
        clean_current_userspace(); // Free the memory associated with the userspace. We could let the next line take care of it, but
                                   // doing it directly is much faster
        pd_paddr_to_free = current_task->page_directory; // Notify the system that the current address space is ready to be cleaned
        kfree(current_task);
    } else {
        // The state is already saved by the IRQ, we just need to update the stack pointer and the eip
        task_queue->esp = esp;

        // Swap the active task
        task_t* queue = task_queue;
        while (queue->next) {
            queue = queue->next;
        }
        queue->next = task_queue;
        task_queue = task_queue->next;
        queue->next->next = 0;
    }


    // Now all we have to do is put in the correct stack and address space, and the program will continue on
    // to return from the interrupt

    set_kernel_stack(task_queue->kernel_esp);
    __asm__ __volatile__("mov %0, %%esp;" :: "r"(task_queue->esp));
    __asm__ __volatile__("mov %0, %%cr3;" :: "r"(task_queue->page_directory));

    // Now that we've moved into the new addresss space, we can clean the old one if needed
    if (pd_paddr_to_free) {
        free_pd(pd_paddr_to_free);
        pd_paddr_to_free = 0;
        //printf("allocated byes: %x\n", get_allocated_bytes(0));
        //printf("free blocks: %x\n", pmmngr_get_free_block_count());
    }

    __asm__ __volatile__("sti"); // Reenable interrupts

    // And now we're officially running from the old task!

    // Now when the IRQ returns, it will be running where the last process left off!

    /*
    // Read esp, ebp now for saving later on.
    uint32_t esp, ebp, eip;
    __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));
    __asm__ __volatile__("mov %%ebp, %0" : "=r"(ebp));

    // We also need to get the eip
    eip = read_eip();
    printf("starting at read_eip: %x\n", eip);

    // Again, here we could be either after a tast switch, or right now
    if (eip == 0x12345) { // This will signal that we're the task that is just resuming
        printf("eip: %x\n", eip);
        return;
    }

    // Now we're the same task that was running initially
    task_queue->eip = eip;
    task_queue->esp = esp;
    task_queue->ebp = ebp;
    
    task_t* queue = task_queue;
    while (queue->next) {
        queue = queue->next;
    }
    queue->next = task_queue;
    task_queue = task_queue->next;

    //printf("esp: %x, ebp: %x, eip: %x\n", task_queue->esp, task_queue->ebp, task_queue->eip);

    set_regs(task_queue->esp, task_queue->ebp, task_queue->eip, task_queue->page_directory);
    */

    // Here we:
   // * Stop interrupts so we don't get interrupted.
   // * Temporarily put the new EIP location in ECX.
   // * Load the stack and base pointers from the new task struct.
   // * Change page directory to the physical address (physicalAddr) of the new directory.
   // * Put a dummy value (0x12345) in EAX so that above we can recognise that we've just
   // switched task.
   // * Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
   // the next instruction.
   // * Jump to the location in ECX (remember we put the new EIP in there).
   

   /*__asm__ __volatile__("cli;" ::);
   __asm__ __volatile__("mov %0, %%esp;
   __asm__ __volatile__("
   __asm__ __volatile__("
   __asm__ __volatile__("
   __asm__ __volatile__("
   __asm__ __volatile__("
      cli;                 \
      mov %0, %%esp;       \
      mov %1, %%ebp;       \
      mov %2, %%cr3;       \
      mov $0x12345, %%eax; \
      mov %3, %%ecx;       \
      sti;                 \
      jmp *%%ecx;" : : "r"(task_queue->esp), "r"(task_queue->ebp), "r"(task_queue->page_directory), "r"(task_queue->eip));


   __asm__ __volatile__("mov %0, %%cr3;" : :"r"(task_queue->page_directory));

   __asm__ __volatile__(" \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx;           ": : );*/
}

void init_multitasking(uint32_t initial_pd_physical_addr) {
    __asm__ __volatile__("cli"); // Disable interrupts
    // Move the stack to outside of the kernel's code or heap. This way the stack will be copied for a new task
    move_stack((void*)0xE0000000, 0x2000);

    task_queue = kmalloc(sizeof(task_t));
    task_queue->id = next_pid++;
    //task_queue->esp = 0;
    //task_queue->ebp = 0;
    //task_queue->eip = 0;
    task_queue->page_directory = initial_pd_physical_addr;
    //task_queue->next = 0;

    __asm__ __volatile__("sti"); // Re-enable interrupt
}
