#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

typedef struct {
    //char *name;
    //char *path;
    char letter;
    unsigned char type;
    void* file; // We will leave this for now, but I think later fs_file will need to move to 
                    // fat32 specifically and this will have to be a void* or something, for general access

    //unsigned int cluster; // Cluster in fat32
    //unsigned int size; // in bytes
    //unsigned int parent; // pointer to another fs_node
} fs_node;

typedef struct node_listt {
    fs_node* node;
    struct node_listt* next;
} node_list;

typedef fs_node *(*finddir_t)(unsigned char drive, unsigned int sec, void *vbr, fs_node *dir, char *name);
typedef unsigned char (*readblock_t)(unsigned char drive, unsigned int sec, void *vbr, fs_node *file, void *buffer);
typedef unsigned char (*advblock_t)(unsigned char drive, unsigned int sec, void *vbr, fs_node *file, int num_blocks);
typedef void *(*get_br_t)(unsigned char drive, unsigned int sector);
typedef fs_node *(*get_rn_t)(void *bootrecord);
typedef int (*openfile_t)(unsigned char drive, unsigned int sec, void *vbr, fs_node *file);
typedef void (*freenode_t)(fs_node *file);
typedef unsigned int (*getblocksize_t)(void* vbr);

typedef struct {
    char *name;
    finddir_t finddir;
    //readdir_type_t readdir;
    readblock_t readblock;
    advblock_t advblock;
    //write_block_type_t write_block;
    //update_direntry_size_type_t update_size;
    //create_file_type_t create_file;
    //delete_file_type_t delete_file;
    get_br_t get_br;
    get_rn_t get_rn;
    openfile_t openfile;
    freenode_t freenode;
    getblocksize_t getblocksize;
} fs_type;

fs_type fat32_type;

int open(const char* path);
int close(int fd);
int get_block_size(int fd);
int read(int fd, unsigned char* buffer);
int adv_buffer(int fd, int num);
int mount(unsigned char drive, unsigned char pnum, char letter);
int unmount(char letter);

#endif
