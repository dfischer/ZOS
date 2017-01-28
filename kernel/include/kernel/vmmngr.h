#ifndef _KERNEL_VMMNGR_H
#define _KERNEL_VMMNGR_H

#include <stdint.h>

uint32_t* get_pagedir(void);
uint32_t* get_pagetable(uint32_t);
void* allocate_page(void);
int allocate_specific_page(uint32_t, int, int);
void free_page(void*);

void* allocate_pages(int);
void free_pages(void*, int);

uint32_t clone_current_pd();
void clean_current_userspace();

void free_pd(uint32_t pd_physical);

void* get_virtual_addr(uint32_t physical_addr);

#endif
