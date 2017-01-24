# Memory organization

After paging has been enabled, the GDT is setup, and the identity map is removed, the code that is linked at around 1MB, the kernel code, is mapped to 0xC0000000.

The last 4MB of memory, i.e. the last page table, is mapped to the page directory itself. This means that by accessing the memory in the last 4 KB the page directory can be modified, and by accessing the memory in the last 4 MB each page table can be modified. 

## Heap

The heap implicitly ranges from the end of the end of the kernel to 0xE0000000, and is allocated one page at a time. This means that if you try to access memory that has not been allocated a page fault will occur. 

## Stack

The stack starts out at the end of the kernel data section, but we then transfer it to 0xE0000000. The reason for doing this is that it means we know the memory from 0xC0000000 to 0xE0000000 will need to be linked when a new address space is created. The memory from 0xE0000000 to the end, minus the last 4 MB will need to be copied, so that the stack is copied when a new process is forked.