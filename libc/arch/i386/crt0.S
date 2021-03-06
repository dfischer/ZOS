.section .text

.global _start
_start:
    pop %eax # argc
    pop %ecx # argv

    # Set up end of the stack frame linked list.
    movl $0, %ebp
    pushl %ebp # eip=0
    pushl %ebp # ebp=0
    movl %esp, %ebp

    # We need those in a moment when we call main.
    pushl %ecx # argv
    pushl %eax # argc

    # Prepare signals, memory allocation, stdio and such.
    #call initialize_standard_library

    # Run the global constructors.
    call _init

    # Restore argc and argv.
    #popl %ecx
    #popl %eax

    # Run main
    call main

    # Terminate the process with the exit code.
    push %eax
    call exit
.size _start, . - _start
