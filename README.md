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

* Look more into where everything really is stored in memory

* Learn about the GDT (http://wiki.osdev.org/GDT) and how to implement it (http://wiki.osdev.org/GDT_Tutorial)

* Learn about interrupts and how to implement them as well

* Paging (maybe take from old os)

* Memory manager (maybe take from old os)

* ATA driver (maybe take from old os)

* FAT32 driver

* Userspace

