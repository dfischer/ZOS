#ifndef _KERNEL_READ_ELF_H
#define _KERNEL_READ_ELF_H

#include <stdint.h>

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
} __attribute__((packed)) pheader_entry;

typedef struct {
    uint32_t name_offset;
    uint32_t type;
    uint32_t attr;
    uint32_t vaddr;
    uint32_t offset;
    uint32_t size; // in bytes
    uint32_t link;
    uint32_t info;
    uint32_t alignment;
    uint32_t ent_size;
} __attribute__((packed)) sheader_entry;

//int read_elf(char* filename);

#endif
