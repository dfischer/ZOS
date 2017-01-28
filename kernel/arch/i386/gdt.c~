#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include<stdio.h>

struct gdt_entry {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

struct tss_entry_struct {
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unusued now.. 
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         
   uint32_t cs;        
   uint32_t ss;        
   uint32_t ds;        
   uint32_t fs;       
   uint32_t gs;         
   uint32_t ldt;      
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed));
typedef struct tss_entry_struct tss_entry_t;

struct gdt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[6];
struct gdt_ptr gp;

tss_entry_t tss_entry;

extern void gdt_flush();
extern void tss_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;
	
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = (limit >> 16) & 0x0F;
	
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

void gdt_install() {
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (int)&gdt;
	
	gdt_set_gate(0, 0, 0, 0, 0); // NULL entry
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // ring 0 code
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // ring 0 data
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // ring 3 code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // ring 3 data

    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = sizeof(tss_entry);

	unsigned char access = 0xE9;
    unsigned char gran = 0x00;

    gdt_set_gate(5, base, limit, access, gran); // TSS

    memset(&tss_entry, 0, sizeof(tss_entry_t)); // Just make sure it's 0
    tss_entry.ss0 = 0x10;
    tss_entry.esp0 = 0;
    tss_entry.cs = 0x0B;
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;

	gdt_flush();
    tss_flush();
}

void set_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}
