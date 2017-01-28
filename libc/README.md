# Standard Library

This includes the files needed to compile the standard c library, libc. Most of these files are also used in the kernel version of libc which is compiled at the same time, libk. The difference comes in which files are included in the library (crt0.S is currently the only difference, but it seems like at this point that needs to be included as a standalone file anyway...), and in the source code with `#if defined(__is_libk)` or `#if defined(__is_libc)`. The difference is that the libk code calls kernel functions, while the libc code uses system calls to the same kernel functions.

## Contents

Most of the code here could be taken directly from free online libraries of libc, and in fact I should probably include more of those files. Some of the OS specific parts, however, that almost always involve system calls, need to be handled correctly. For these, you should make a syscall in stdlib/syscall.c that performs the required action. If necessary, more functions should be added to the kernel.

## Build

Both libk and libc are built at the same time, the only difference is that libk has the freestanding flag, some included files could be different, and the libk output files are moved from .o to .libk.o so they do not conflict with the same files for libc.