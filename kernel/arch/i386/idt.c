#include <string.h>

struct idt_entry {
	unsigned short base_low;
	unsigned short sel;
	unsigned char always0;
	unsigned char flags;
	unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
	idt[num].base_low = base & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	
	idt[num].sel = sel;
	idt[num].flags = flags;
	idt[num].always0 = 0;
}

void idt_install() {
	idtp.limit = (sizeof(struct idt_entry) * 256) -1;
	idtp.base = (unsigned int) &idt;
	
	memset(&idt, 0, sizeof(struct idt_entry) * 256);
	
	idt_load();
}

unsigned char inportb(unsigned short _port) {
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void inportsw(unsigned short _port, unsigned short *_buffer, unsigned int _words) {
    __asm__ __volatile__ ("rep insw" : : "c" (_words), "d" (_port), "D" (_buffer));
}

void outportsw(unsigned short _port, unsigned short *_buffer, unsigned int _words) {
    for (unsigned int i = 0; i < _words; i++) {
        unsigned short data = _buffer[i];
        __asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (data));
    }
}

void outportb(unsigned short _port, unsigned char _data) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));

}
