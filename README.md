# ZOS

This is, at this point, a functioning operating system. There are a number of things that need to be implemented for it to be actually useful, but the fundamental components exist. In my opinion, these are the ability to read from disk, execute ELF files, and run multiple programs in user space.

## Components

Right now there are two projects that get built that together constitute ZOS:

1. The kernel. This is the heart of the operating system, and contains the logic that defines how memory is allocated, what instructions are executed, how to access disk, etc. This is compiled into myos.kernel along with libk.a, forming the ELF executable that grub loads. This runs in the freestanding mode (-ffreestanding), which means that it does not expect a standard library.

2. The standard library. This actually two separate library files that share most of their code, libc.a and libk.a. There are a number of operations that are convenient to use in the kernel code, such as memcpy, memset, strcmp, etc. that are a part of the c standard library. There are also functions like printf that we want to behave the same way as in hosted mode, but they must be implmented with knowledge of the kernel functions that actually write to the terminal. Because of this, libk.a is first built and linked into the kernel code, while libc.a is built separately and placed into sysroot/usr/lib. When programs are compiled in hosted mode, they automatically link libc.a from sysroot/usr/lib. There is actually a third binary file as well, crt0.o, which gets placed in sysroot/usr/lib, which is needed for c programs to run.

## Build system

The build system is made up of a number of bash scripts that set environment variables and direct the build, and a Makefile for each project, in the respective project folder. Right now there are two ways to build and run, with the kernel loaded in an iso (./qemu.sh), or with the kernel loaded onto a disk img file (sudo ./mrun_from_disk.sh). The run from disk will work, but you may need to run ./build first with non-sudo to get it to work. I would suggest the iso version for now.

## Folder information

* kernel - contains the kernel project
* libc - contains the libc and libk projects
* os_specific_toolchain - contains the ZOS specific toolchain (http://wiki.osdev.org/OS_Specific_Toolchain). I have installed this to $HOME/opt/zos_cross, and it replaces the initial cross-compiler. Because I used mainly default values, the functionality is similar to the initial cross-compiler, but because I included the libc headers in sysroot/usr/include it can compile the kernel with -ffreestanding and user space hosted programs with just normal flags
* sysroot - The system root folder, for now it is a way to organize the binaries, but it also serves as the system root for i686-zos-gcc. Instead of looking in /usr/lib for libc.a for example, i686-zos-gcc will look in ./sysroot/usr/lib. Ultimately it will serve as the / folder in the file system
* isodir - Created during the build process, grub turns this folder into a iso image that can be loaded. The kernel is copied here, along with the grub config file
* test_prog - Holds a simple test program that gets compiled in hosted mode and run from disk by the OS

## Memory Organization

The kernel is loaded in the higher half at virtual address 0xC0000000 to allow user space programs to execute below that. This requires a trick with the GDT before paging is enabled, but after that everything works well. 

Near the beginning of execution, the stack is moved from the .bss section in the kernel data to 0xE0000000. After this, the kernel code and data always start at 0xC0100000, the kernel heap starts directly after the kernel data, and the kernel stack starts at 0xE0000000 + size and grows downwards. The kernel code, data, and heap are shared between all processes at the same position in memory. For each process, a new stack is created and mapped to 0xE0000000 in the process's address space. This is the stack that is used when an interrupt happens in the user space operation of the process, for example, and it is important that the kernel stacks be separate between processes.

When the OS loads a program into memory, it places it starting near 0x08048000, as directed by the ELF file. It then adds a stack and a heap directly after the data segment. The stack is used correctly, but as of now the heap is never used because a user space malloc and free do not exist yet. The user space program uses this stack space, and when an interrupt happens the stack is switched to the process's specific kernel stack. Because of this it's helpful to think about the kernel not as a single process that gets run when an interrupt happens, but as a number of processes, with one existing for each user space process. When an interrupt happens, execution is handed over to the kernel version of the process. When the processes switch, both the user space and the kernel space processes are switched. The user space and kernel space processes effectively share an address space.

## Useful links

* [osdev](http://wiki.osdev.org/)
* [ELF format information](http://www.skyfree.org/linux/references/ELF_Format.pdf)
* [More in-depth examples](http://www.jamesmolloy.co.uk/tutorial_html/) - Note: while these are good as examples, some of the code has bugs and some of it is really not ideal. I really believe my code makes more sense


## Todo

* libc
* more syscalls, in particular malloc and free
* memory permission while paging (user vs kernel)
* complete fat32 driver, include support for writing
* graphics mode
