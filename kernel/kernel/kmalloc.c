#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/vmmngr.h>
#include <kernel/kmalloc.h>

#define HEADER_SIZE 2

static uint8_t *first_page;

uint16_t *read_short(uint8_t *page, uint16_t offset) {
	return (uint16_t *)&page[offset];
}

void write_short(uint8_t *page, uint16_t offset, uint16_t val) {
	*((uint16_t *)&page[offset]) = val;
}


void debug(uint8_t *page, int l) {
	for (int i = 0; i < l; i++) {
		printf("%x ", page[i]);
	}
	printf("\n");
}

void consolidate_space(uint8_t *current_page, uint16_t *header) {
	if (*header & 0x8000) return;
	while (1) {
		uint16_t next_header_addr = *header & 0x0FFF;
		uint16_t *next_header = read_short(current_page, next_header_addr);
		if (*next_header == 0) break;
		uint16_t next_used = *next_header & 0x8000;
		
		if (!next_used) {
			uint16_t next_next_header_addr = *next_header & 0x0FFF;
			*header = next_next_header_addr;
			memset(next_header, 0, 2);
		} else break;
	}
}

uint8_t *setup_page() {
	uint8_t *new_page = allocate_page();
	write_short(new_page, 0, 4090);
	return new_page;
}

void *kmalloc(uint16_t bytes) {
	if (bytes > 4088) {
		printf("Error, allocating too much space, use pages instead\n");
		return 0;
	}
	
	uint8_t *current_page = first_page;
	while(1) {
		uint16_t *header = read_short(current_page, 0);
		while (*header) {
			uint16_t used = *header & 0x8000;
			uint16_t next_header_addr = *header & 0x0FFF;
			uint16_t current_header_addr = (uint32_t)header - (uint32_t)current_page;
		
			if (!used && next_header_addr - (current_header_addr + 2) >= bytes) {
				uint16_t new_end = current_header_addr + 2 + bytes;
				if (next_header_addr - new_end >= 2) {
					*header = 0x8000 | new_end;
					write_short(current_page, new_end, next_header_addr);
				} else {
					*header |= 0x8000;
				}
				//printk("Space found, status of page: \n");
				//debug(current_page, 4096);
				return header+1;
			} else {
				header = read_short(current_page, next_header_addr);
			}
		}
	
		uint32_t *next_page_addr = (uint32_t *)(header+1);
		if (*next_page_addr == 0) {
			uint8_t *next_page = setup_page();
			*next_page_addr = (uint32_t)next_page;
			//printk("next page not found, starting new page at %x\n", *next_page_addr);
		}
		current_page = (uint8_t *)*next_page_addr;
	}
}

uint8_t kfree(void *ptr) {
	uint8_t *current_page = first_page;
	while(1) {
		if ((uint32_t)ptr >= (uint32_t)current_page && (uint32_t)ptr < 4096 + (uint32_t)current_page) break;
		uint16_t *header = read_short(current_page, 4090);
		if (*header != 0) {
			printf("Error in kfree, next page header is not 0");
			return 2;
		}
		
		uint32_t next_page_addr = *(uint32_t *)(header+1);
		if (next_page_addr == 0) {
			printf("Error, pointer not in kmalloc space\n");
			return 1;
		}
		
		current_page = (uint8_t *)next_page_addr;
	}
	//printk("Found page of ptr at %x\n", current_page);
	
	uint16_t *header = (uint16_t *)((uint8_t*)ptr-2);
	uint16_t next_header_addr = *header & 0x0FFF;
	uint16_t size = next_header_addr - ((uint32_t)header - (uint32_t)current_page + 2);
	//printk("Freeing %d bytes of memory at %x\n", size, ptr);
	memset(ptr, 0, size);
	
	*header &= 0x7FFF;
	consolidate_space(current_page, header);

    // Now if this page is empty, then we should really free the unneeded memory. This would require storing the previous page in addition to the last page, though, so for now we're not going to do this.
    // If you do want to do this, you have to check if the current page is now empty, and if so free the page and then remove the pointer to the page from the previous page. THEN you also need to go through that page and see if it's empty, and do the same thing. Keep doing that until you get to a page that is not empty.

	return 0;
}

void kmalloc_init() {
	first_page = setup_page();
	//printf("kmalloc page at %x\n", (uint32_t)first_page);
	
	/*char *p = kmalloc(4088);
	char *p2 = kmalloc(5);
    p2[0] = 0x3C;
	kfree(p);
	kfree(p2);
	
    printf("p2 at: %x with value: %x\n", p2, *p2);

	for (int i = 4080; i < 4096; i++) {
		printf("%x ", first_page[i]);
	}
	printf("\n");*/

	/*char *p = kmalloc(20);
	p[0] = 'H';
	p[1] = 'e';
	p[2] = 'l';
	printf("Allocated 20 bytes at %x\n", p);
	
	char *p2 = kmalloc(6);
	p2[0] = 0xFF;
	p2[1] = 0xFF;
	p2[2] = 0xFF;
	printf("Allocated 6 bytes at %x\n", p2);
	
	char *p3 = kmalloc(3);
	p3[0] = 0x0F;
	p3[1] = 0x0F;
	p3[2] = 0x0F;
	printf("Allocated 3 bytes at %x\n", p3);
	
	//debug(first_page, 50);
	
	kfree(p);
    //p = kmalloc(20);

	debug(first_page, 50);
	
	p = kmalloc(10);
	printf("Allocated 10 bytes at %x\n", p);
	kfree(p3);
	kfree(p2);
	
	
	debug(first_page, 50);
	kfree(p);
	debug(first_page, 50);*/
}
