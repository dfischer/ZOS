#ifndef _DRIVERS_FAT32_H
#define _DRIVERS_FAT32_H

//#include <drivers/mbr.h>


/*typedef struct {
    unsigned char jmp[3];
    unsigned char oemid[8];
    unsigned short bytes_per_sec;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char num_fat_tables;
    unsigned short num_directory_entries;
    unsigned short total_sectors;
    unsigned char media_descriptor;
    unsigned short sectors_per_FAT;
    unsigned short sectors_per_track;
    unsigned short heads;
    unsigned int hidden_sectors;
    unsigned int sectors_ext;
    unsigned int sectors_per_fat;
    unsigned short flags;
    unsigned short version;
    unsigned int root_cluster;
    unsigned short fsinfo_cluster;
    unsigned short bkup_boot_sector_cluster;
    unsigned char reserved[12];
    unsigned char drive_num;
    unsigned char reserved2;
    unsigned char sig;
    unsigned int volumeid;
    unsigned char label[11];
    unsigned char sysid_string[8];
    unsigned char bootcode[420];
    unsigned short partition_sig;
} __attribute__((packed)) vbr32_t;*/

/*typedef struct fat32_nodee {
    char *name;
    //const char *path;
    unsigned char type;
    partition* cpart;
    vbr32_t* vbr;
    unsigned int cluster; // Cluster in fat32
    unsigned int size; // in bytes
    struct fat32_nodee* parent; // pointer to another fs_node. May run into issues here if we free the parent..
    struct fat32_nodee* next; // used to make a list of nodes, if desired
} fat32_node;*/

/*typedef struct fat32_filee {
    unsigned int cluster;
    struct fat32_filee* prev;
    struct fat32_filee* next;
} fat32_file;*/

/*fat32_file* openfile_fat32(fat32_node* file);
unsigned char readblock_fat32(fat32_node* node, fat32_file** file, void* buffer);
void free_fat32_file(fat32_file* file);

fat32_node* readdir_fat32(fat32_node* directory);
void free_nodelist(fat32_node* list);*/

// We shouldn't need to be able to see this
//fat32_node *get_rn_fat32(vbr32_t* vbr);

void init_fat32();

#endif
