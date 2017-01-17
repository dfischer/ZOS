#ifndef _KERNEL_PMMNGR_H
#define _KERNEL_PMMNGR_H

#include <kernel/multiboot.h>
#include <stdint.h>

#define PMMNGR_BLOCKS_PER_BYTE	8
#define PMMNGR_BLOCK_SIZE		4096
#define PMMNGR_BLOCK_ALIGN		PMMNGR_BLOCK_SIZE

void pmmngr_init(multiboot_info_t*, uint32_t);
uint32_t pmmngr_alloc_block(void);
void pmmngr_free_block(uint32_t addr);
uint32_t pmmngr_get_free_block_count(void);
void print_map(uint32_t, uint32_t);

#endif
