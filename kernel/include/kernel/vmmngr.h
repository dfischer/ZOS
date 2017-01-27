#ifndef _KERNEL_VMMNGR_H
#define _KERNEL_VMMNGR_H

#include <stdint.h>

uint32_t* get_pagedir(void);
uint32_t* get_pagetable(uint32_t);
void* allocate_page(void);
void* allocate_specific_page(uint32_t);
void free_page(void*);

void* allocate_pages(int);
void free_pages(void*, int);

uint32_t clone_current_pd();
void free_pd(uint32_t pd_physical);

void* get_virtual_addr(uint32_t physical_addr);

#endif
