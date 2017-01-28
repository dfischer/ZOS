# How to debug with qemu and gdb

1. Run qemu in debug mode with ./debug.sh

This will set up qemu to listen for the gdb debugger, and also stop execution so it doesn't run.

2. Run gdb with ./opengdb.sh to use the symbols from the kernel, or ./opengdb.sh2 to use symbols from test_prog/test

3. Create a breakpoint with break <function name\> or break <file name\>:<line number\>

4. Run with c

## Commands

* c = continue
* s = execute one line
* si = execute one assembly function
* n = execute one line without stepping into functions