# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate the initial stack.
.section .bss, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
    lgdt trickgdt
    
    movw $0x0010, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    #ljmp $0x0008, $higherhalf
#higherhalf:
    # Set up the stack.
    mov $stack_top, %esp

    # Call the global constructors
    call _init

    # Enter the high-level kernel.
    call kernel_main

    # Infinite loop if the system has nothing more to do.
    cli
1:  hlt
    jmp 1b
.size _start, . - _start

.global gdt_flush
.extern gp
gdt_flush:
    lgdt gp
    movw $0x0010, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x0008, $flush2
flush2:
    ret

.global loadPageDirectory
loadPageDirectory:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    mov %eax, %cr3
    mov %ebp, %esp
    pop %ebp
    ret

.global enablePaging
enablePaging:
    push %ebp
    mov %esp, %ebp
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0
    lea 2f, %ecx
    jmp *%ecx
2:
    mov %ebp, %esp
    pop %ebp
    ret

.section .setup

trickgdt:
    .word gdt_end - gdt - 1;
    .long gdt;

gdt:
    .word 0x0000, 0x0000, 0x0000, 0x0000
    .byte 0xFF, 0xFF, 0, 0, 0, 0b10011010, 0b11001111, 0x40
    .byte 0xFF, 0xFF, 0, 0, 0, 0b10010010, 0b11001111, 0x40
gdt_end:
