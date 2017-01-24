#include <stdint.h>
#include <stdio.h>

#include <kernel/vfs.h>
#include <kernel/vmmngr.h>

typedef struct {
    uint32_t magic;
    uint8_t address_size;
    uint8_t endian;
    uint8_t elf_version;
    uint8_t os_abi;
    uint8_t padding[8];
    uint16_t exe_type;
    uint16_t iset;
    uint32_t elf_version2;
    uint32_t pentry_pos;
    uint32_t program_header_table_pos;
    uint32_t section_header_table_pos;
    uint32_t flags;
    uint16_t header_size;
    uint16_t p_header_table_entry_size;
    uint16_t p_header_table_num_entries;
    uint16_t s_header_table_entry_size;
    uint16_t s_header_table_num_entries;
    uint16_t section_name_pos;
} __attribute__((packed)) elf_header;

typedef struct {
    uint32_t type;
    uint32_t data_offset;
    uint32_t vaddr;
    uint32_t undefined;
    uint32_t segment_size_file;
    uint32_t segment_size_memory;
    uint32_t flags;
    uint32_t alignment;
} __attribute__((packed)) header_entry;

int read_elf(char* filename) {
    int fd = open(filename);
    unsigned char* buffer = allocate_page();
    read(fd, buffer);
    elf_header* header = (elf_header*) buffer;
    printf("magic: %x, type: %x, entry_pos: %x, header table pos: %x, section table pos: %x\n", header->magic, header->exe_type, header->program_header_table_pos, header->section_header_table_pos);
    
    int res = close(fd);
    return res;
}
