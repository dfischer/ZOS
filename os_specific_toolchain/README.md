# OS-Specific Toolchain

In order to compile programs to be run with ZOS, a slightly specific cross-compiler is needed, which knows what libc functions are available. To build the specific cross compiler, the libc headers are needed, so if those ever drastically change (they shouldn't, they're supposed to be standard), then the cross-compiler should be recompiled. 

## Differences

I basically followed [this](http://wiki.osdev.org/OS_Specific_Toolchain) tutorial exactly, to makes the changes to gcc and binutils needed.

## Building

To actually build the cross compiler, I followed the instructions [here](http://wiki.osdev.org/Hosted_GCC_Cross-Compiler). After performing these steps (and first looking through [here](http://wiki.osdev.org/GCC_Cross-Compiler) to make sure I knew what was happening), I created a cross compiler, where gcc is for example i686-zos-gcc.

## Use

Once gcc has been compiled, usermode c programs can be compiled just like normal: `i686-zos-gcc -c file.c -o file`. An example of this is in test_prog/Makefile. Freestanding programs (the kernel) can also be compiled by just adding the -ffreestanding flag.

## Goal

Ultimately, I want to have a large enough libc that I can compile the cross-compiler itself and add it to sysroot. At that point, along with an editor, I could move all of the development to ZOS (in theory, though debugging would probably be harder).