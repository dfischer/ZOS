#include <stdint.h>
#include <stdio.h>

#include <kernel/vfs.h>
#include <kernel/vmmngr.h>
#include <kernel/kmalloc.h>

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

char* open_strtab(FILE* file, elf_header* header) {
    sheader_entry* strtab_header = kmalloc(sizeof(sheader_entry));
    int strtab_filepos = header->section_header_table_pos+header->section_name_pos*header->s_header_table_entry_size;
    printf("strtab position: %x\n", strtab_filepos);
    fseek(file, header->section_header_table_pos+header->section_name_pos*header->s_header_table_entry_size, SEEK_SET);
    fread(strtab_header, sizeof(sheader_entry), 1, file);

    char* string_table = kmalloc(strtab_header->size+1);
    fseek(file, strtab_header->offset, SEEK_SET);
    fread(string_table, strtab_header->size, 1, file);

    kfree(strtab_header);
    return string_table;
}

char* read_strtab(char* strtab, int idx) {
    return strtab+idx;
}

void close_strtab(char* string_table) {
    kfree(string_table);
}

int kexecv_elf(const char* filename, char* const argv[]) {
    FILE* file = fopen(filename, "r");

    if (!file) {
        printf("Error, unable to open file %s\n", filename);
        return 1;
    }

    elf_header* header = (elf_header*) kmalloc(sizeof(elf_header));
    fread(header, sizeof(elf_header), 1, file);

    if (header->magic != 0x464C457F) {
        printf("Error, the file is not an ELF executable, the magic bits are: %x\n", header->magic);
        return 2;
    }

    //printf("magic: %x, type: %x, entry_pos: %x, header table pos: %x, section table pos: %x header size: %x, header table entries: %x, section table entries: %x, section_name_pos: %x, ptable_esize: %x, stable_esize: %x\n", header->magic, header->exe_type, header->pentry_pos, header->program_header_table_pos, header->section_header_table_pos, header->header_size, header->p_header_table_num_entries, header->s_header_table_num_entries, header->section_name_pos, header->p_header_table_entry_size, header->s_header_table_entry_size);
    

    pheader_entry* pheader_table = (pheader_entry*) kmalloc(sizeof(pheader_entry)*header->p_header_table_num_entries);

    fseek(file, header->program_header_table_pos, SEEK_SET);
    fread(pheader_table, sizeof(pheader_entry), header->p_header_table_num_entries, file);

    for (int i = 0; i < header->p_header_table_num_entries; i++) {
        pheader_entry* ent = pheader_table+i;
    
        if (ent->type == 0x01) { // load
            
        } else {
            printf("error, unable to handle program header table entry: type: %x, offset: %x, vaddr: %x, size_file: %x, size_mem: %x, flags: %x, align: %x\n", ent->type, ent->data_offset, ent->vaddr,  ent->segment_size_file,  ent->segment_size_memory,  ent->flags,  ent->alignment);
        }
    }
    
    kfree(pheader_table);
    kfree(header);
    fclose(file);


}

int read_elf(char* filename) {
    
    FILE* file = fopen(filename, "r");
    elf_header* header = (elf_header*) kmalloc(sizeof(elf_header));
    fread(header, sizeof(elf_header), 1, file);

    printf("magic: %x, type: %x, entry_pos: %x, header table pos: %x, section table pos: %x header size: %x, header table entries: %x, section table entries: %x, section_name_pos: %x, ptable_esize: %x, stable_esize: %x\n", header->magic, header->exe_type, header->pentry_pos, header->program_header_table_pos, header->section_header_table_pos, header->header_size, header->p_header_table_num_entries, header->s_header_table_num_entries, header->section_name_pos, header->p_header_table_entry_size, header->s_header_table_entry_size);
    

    
    pheader_entry* pheader_table = (pheader_entry*) kmalloc(sizeof(pheader_entry)*header->p_header_table_num_entries);

    fseek(file, header->program_header_table_pos, SEEK_SET);
    fread(pheader_table, sizeof(pheader_entry), header->p_header_table_num_entries, file);

    for (int i = 0; i < header->p_header_table_num_entries; i++) {
        pheader_entry* ent = pheader_table+i;
        printf("type: %x, offset: %x, vaddr: %x, size_file: %x, size_mem: %x, flags: %x, align: %x\n", ent->type, ent->data_offset, ent->vaddr,  ent->segment_size_file,  ent->segment_size_memory,  ent->flags,  ent->alignment);
    }
    
    kfree(pheader_table);
    printf("\n");


    char* string_table = open_strtab(file, header);

    sheader_entry* sheader_table = (sheader_entry*) kmalloc(sizeof(sheader_entry)*header->s_header_table_num_entries);

    fseek(file, header->section_header_table_pos, SEEK_SET);
    fread(sheader_table, sizeof(sheader_entry), header->s_header_table_num_entries, file);

    for (int i = 0x13; i < header->s_header_table_num_entries; i++) {
        sheader_entry* ent = sheader_table+i;
        printf("name: %s, type: %x, attr: %x, vaddr: %x, offset: %x, size: %x, info: %x\n", read_strtab(string_table, ent->name_offset), ent->type, ent->offset,  ent->vaddr,  ent->offset,  ent->size,  ent->info);
    }

    close_strtab(string_table);
    kfree(sheader_table);

    kfree(header);
    int res = fclose(file);


    return res;

    /*int fd = open(filename);
    unsigned int* buffer = allocate_page();
    read(fd, (unsigned char*)buffer);
    read(fd, (unsigned char*)buffer);
    //read(fd, (unsigned char*)buffer);
    for (int i = 0; i < 10; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
    free_page(buffer);*/
}
