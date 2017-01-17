# Operating system notes

## Setup

Installing/building a cross-compiler so that the code will not be compiled for Linux: [http://wiki.osdev.org/GCC_Cross-Compiler#The_Build](http://wiki.osdev.org/GCC_Cross-Compiler#The_Build)

Installing dependencies with apt-get, but binutils and the compiler (gcc) from source (so it's not intsalled for Linux)

Using gcc 4.8.4 (my current version), and binutils 2.27

The installed compiler is i686-elf-gcc

I will use C to start, because the tutorials are in C, but ultimately I would like to use C++

# Implemented Components

## Bootloader

Right now I'm using GRUB as the bootloader, which hands off to boot.S. boot.S declares the things necessary for multiboot, reserves space for the stack (in section bss), and includes the code

# Todo

* Make the pages dynamically allocated after you have a working physical memory manager

* Only allocate the pages for kernel, not the whole memory



* Learn about interrupts and how to implement them as well

* Paging (maybe take from old os)

* Memory manager (maybe take from old os)

* ATA driver (maybe take from old os)

* FAT32 driver

* Userspace

# Not sure of yet:

Do I need to jump to the higher half?

Do I need the write-protected bit when enabling paging: 0x80000000 or 0x80010000

Do I need to map the VGA buffer explicitly and leave the address what it is now, or should I initialize all of the pages to 0 and just use 0xC0000000 + whatever it was before (B something)

Do I use the hardware address (PTR version) or virtual address for loadPageDirectory(probably the hardware address because paging isn't set up yet)

