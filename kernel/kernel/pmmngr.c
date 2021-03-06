#include <kernel/multiboot.h>
#include <kernel/pmmngr.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

uint32_t mmngr_memory_size = 0;
uint32_t mmngr_used_blocks = 0;
uint32_t mmngr_max_blocks = 0;

// We will use a 1 mega-bit = 1MB/8 chunk of memory in the kernel for storing the bitmap. This way we don't have to pick
// a space by hand. Not all of it will even exist, so first we will fill it with ones (i.e. it's in use), and then
// we will mark as free all the entries that exist. There are 0x20000 bytes = 0x8000 uint32's
uint32_t mmngr_mmap[0x8000];

uint32_t pmmngr_get_free_block_count () {

	return mmngr_max_blocks - mmngr_used_blocks;
}

void set_mmap(int bit) {
	mmngr_mmap[bit/32] |= 1 << (bit % 32);
}

void unset_mmap(int bit) {
	mmngr_mmap[bit/32] &= ~(1 << (bit % 32));
}

// if the result is > 0, then the bit is set which means the page in question is used
int test(int bit) {
	return mmngr_mmap[bit/32] & (1 << (bit % 32));
}

// Returns the index of the first free block, i.e. the block number
// if no blocks are free, returns -1
int mmap_first_free() {
	int i;
	for (i = 0; i < mmngr_max_blocks / 32; i++) {
		if (mmngr_mmap[i] == 0xFFFFFFFF) continue;
		int j;
		for (j = 0; j < 32; j++) {
			if (!(mmngr_mmap[i] & (1 << j))) {
				return i*32+j;
			}
		}
	}
	return -1;
}

/*
 * Initializes the physical memory map, taking the multiboot info and the number of pages that should be marked as
 * used because they are used (statically) by the kernel
 */

void pmmngr_init_region(uint32_t base, uint32_t size) {
	uint32_t align = base / PMMNGR_BLOCK_SIZE;
	uint32_t blocks = size / PMMNGR_BLOCK_SIZE; // This will round down, which is what we want
	
	for(; blocks > 0; blocks--) {
		unset_mmap(align++);
		mmngr_used_blocks--;
	}
	
	set_mmap(0); // Not really needed... just make sure that the first section of memory is off limits
}

void pmmngr_deinit_region(uint32_t base, uint32_t size) {
	uint32_t align = base / PMMNGR_BLOCK_SIZE;
	uint32_t blocks = size / PMMNGR_BLOCK_SIZE;
	
	for(; blocks > 0; blocks--) {
        if (test(align) == 0x00000000) {
		    set_mmap(align);
		    mmngr_used_blocks++;
        }
        align++;
	}
}

void pmmngr_init(multiboot_info_t *mbd, uint32_t kernel_pages) {
	uint32_t mem_size = mbd->mem_lower + mbd->mem_upper; // NOTE: I may need a + 1024 here, though I'm not sure why...

	mmngr_memory_size = (uint32_t)mem_size;
	mmngr_max_blocks = 0x100000; // 1 million blocks possible in 4 GB //mmngr_memory_size *1024 / PMMNGR_BLOCK_SIZE+1;
    mmngr_used_blocks = mmngr_max_blocks; // At first they will all be used
	
	memset(mmngr_mmap, 0xFF, 0x20000); // First set them all to be used. This takes the number of bytes
	
    //printf("mmap_addr: %x, mmap_length: %x\n", mbd->mmap_addr, mbd->mmap_length);

    // Next, set the usable regions to be indicated with a 0
	memory_map_t* mmap = (memory_map_t*)mbd->mmap_addr;
	while ((unsigned int)mmap < mbd->mmap_addr + mbd->mmap_length) {
		//printf("mmap entry base address: %x, length: %x, size: %x, type: %d\n", (int)mmap->base_addr, (int)mmap->length, (int)mmap->size, mmap->type);
		
        // If the section is usable, indicate by setting all of the entries to 0
        if (mmap->type == 1 && mmap->base_addr != 0) {
			pmmngr_init_region((uint32_t)mmap->base_addr, (uint32_t)mmap->length);
		}
		
		mmap = (memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int));
	}

	// Now we just need to make sure that the pages that have already been given to the kernel (the pages that are
    // accounted for by the virtual memory manager) are not allowed to be given to others
    pmmngr_deinit_region(0x00000000, kernel_pages*4096);
    
    //printf("free blocks: %d\n", mmngr_max_blocks-mmngr_used_blocks);

    //print_map(0, 20);
}

// This will have to work entirely with ints instead of pointers so that the conversion to virtual addresses 
// never happens
uint32_t pmmngr_alloc_block() {
	if (pmmngr_get_free_block_count() <=0) return 0;
	
	int frame = mmap_first_free();
	if (frame == -1) return 0;
	
	set_mmap(frame);
	uint32_t addr = frame * PMMNGR_BLOCK_SIZE;
	//memset((unsigned char *)addr, 0x00, PMMNGR_BLOCK_SIZE);
	mmngr_used_blocks++;
	
	return addr;
}

// Likewise, this will refer only to ints instead of pointers
void pmmngr_free_block(uint32_t addr) {
	int frame = addr / PMMNGR_BLOCK_SIZE;
	unset_mmap(frame);
	mmngr_used_blocks--;
}

void print_map(uint32_t start, uint32_t end) {
    printf("mmap location: %x\n", mmngr_mmap);
    for (uint32_t i = start; i < end; i++) {
        printf("%x ", mmngr_mmap[i]);
    }
    printf("\n");
}

/*uint32_t pmmngr_get_memory_size () {

	return mmngr_memory_size;
}

uint32_t pmmngr_get_block_count () {

	return mmngr_max_blocks;
}

uint32_t pmmngr_get_used_block_count () {

	return mmngr_used_blocks;
}*/


/*uint32_t pmmngr_get_block_size () {

	return PMMNGR_BLOCK_SIZE;
}*/
