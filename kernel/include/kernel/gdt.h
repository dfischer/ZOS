#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stddef.h>

void gdt_install(void);
void set_kernel_stack(uint32_t stack);

#endif
