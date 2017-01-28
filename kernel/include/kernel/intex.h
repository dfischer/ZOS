#ifndef _KERNEL_INTEX_H
#define _KERNEL_INTEX_H

typedef struct regs {
    unsigned int ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} regs_t;

typedef void (*ihandler_t)(regs_t*);

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void idt_install(void);

void isrs_install(void);

void irq_install_handler(int, ihandler_t);
void irq_uninstall_handler(int);
void irq_install(void);

unsigned char inportb(unsigned short);
void inportsw(unsigned short, unsigned short*, unsigned int);
void outportsw(unsigned short, unsigned short*, unsigned int);
void outportb(unsigned short, unsigned char);


#endif
