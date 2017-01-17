#ifndef _KERNEL_VMMNGR_H
#define _KERNEL_VMMNGR_H

uint32_t* get_pagedir(void);
uint32_t* get_pagetable(uint32_t);
void* allocate_page(void);
void free_page(void*);

#endif