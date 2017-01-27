#include <stdio.h>
#include <string.h>

#include <kernel/kmalloc.h>
#include <drivers/mbr.h>
#include <drivers/ata.h>
#include <kernel/vfs.h>

// Take out when done debugging!
#include <drivers/fat32.h>

typedef struct {
    fs_type *type;
    //fs_node *rootnode;
    void *bootrecord;
    unsigned char drive;
    unsigned int sector;
} mountpoint;

static mountpoint* ext_mp[26];
static fs_node* fd_table[1024];

// This assumes that the path is like a:/foo/bar/testas
int open(const char* path) {
    int i;
    for (i = 0; i < 1024; i++) {
        if (!fd_table[i]) break;
    }
    
    if (i == 1024) {
        printf("Error: no available file descriptors, there are already the max (1024) number of files opened\n");
        return -1;
    }

    char* str_buffer = kmalloc(strlen(path)+1);
    memcpy(str_buffer, path, strlen(path));
    char* letter = strtok(str_buffer, ":");
    char letter_real = letter[0];

    if (!letter || strlen(letter) > 1 || letter_real < 'a' || letter_real > 'z') {
        printf("Error, the drive in the path %s is not a valid drive letter\n", path);
        kfree(str_buffer);
        return -1;
    }

    mountpoint *mp = ext_mp[letter_real-'a'];
    fs_node* curnode = mp->type->get_rn(mp->bootrecord); // This will allocate space for a node

    char* dirname = strtok(NULL, "/");
    while (dirname) {
        fs_node* nextnode = mp->type->finddir(mp->drive, mp->sector, mp->bootrecord, curnode, dirname); // this will put the found node in curnode
        mp->type->freenode(curnode);
        if (!nextnode) {
            printf("Error, unable to find file %s\n", path);
            kfree(str_buffer);
            return -1;
        }
        curnode = nextnode;
        dirname = strtok(NULL, "/");
        
    }
    kfree(str_buffer);

    // The file needs to be a file, not a directory or something else wierd
    if (curnode->type != 0x00) {
        printf("Error, %s exists but is not a file\n", path);
        mp->type->freenode(curnode);
        return -1;
    }
    
    curnode->letter = letter_real;
    mp->type->openfile(mp->drive, mp->sector, mp->bootrecord, curnode);
    fd_table[i] = curnode;
    return i;
}

int close(int fd) {
    if (fd < 0 || fd >= 1024) {
        printf("Error, invalid file descriptor %d, unable to close\n", fd);
        return 0;
    }
    
    fs_node* curnode = fd_table[fd];
    if (!curnode) {
        printf("Error, no file associated with file descriptor %d, unable to close\n", fd);
        return 0;
    }

    fd_table[fd] = 0;
    mountpoint *mp = ext_mp[curnode->letter-'a'];
    printf("curnode letter: %x, curnode type: %x, curnode file: %x, curnode file cluster: %d\n", curnode->letter, curnode->type, curnode->file, ((unsigned int*)curnode->file)[0]);
    mp->type->freenode(curnode);
    return 1;
}

int get_block_size(int fd) {
    if (fd < 0 || fd >= 1024) {
        printf("Error, invalid file descriptor %d, unable to get_block_size\n", fd);
        return -1;
    }

    fs_node* curnode = fd_table[fd];
    if (!curnode) {
        printf("Error, no file associated with file descriptor %d, unable to get block size\n", fd);
        return -1;
    }

    mountpoint *mp = ext_mp[curnode->letter-'a'];
    return mp->type->getblocksize(mp->bootrecord);
}

int read(int fd, unsigned char* buffer) {
    if (fd < 0 || fd >= 1024) {
        printf("Error, invalid file descriptor %d, unable to read\n", fd);
        return 2;
    }

    fs_node* curnode = fd_table[fd];
    if (!curnode) {
        printf("Error, no file associated with file descriptor %d, unable to read\n", fd);
        return 1;
    }

    mountpoint *mp = ext_mp[curnode->letter-'a'];
    int err = mp->type->readblock(mp->drive, mp->sector, mp->bootrecord, curnode, buffer);
    // This should automatically advance the coarse buffer for the file
    return err;
}

int adv_buffer(int fd, int num) {   
    if (fd < 0 || fd >= 1024) {
        printf("Error, invalid file descriptor %d, unable to advance buffer\n", fd);
        return 2;
    }

    fs_node* curnode = fd_table[fd];
    if (!curnode) {
        printf("Error, no file associated with file descriptor %d, unable to advance buffer\n", fd);
        return 1;
    }

    mountpoint *mp = ext_mp[curnode->letter-'a'];
    int err = mp->type->advblock(mp->drive, mp->sector, mp->bootrecord, curnode, num);
    return err;
}

int mount(unsigned char drive, unsigned char pnum, char letter) {
    if (letter < 'a' || letter > 'z') {
        printf("Invalid mount point %c. Must be a letter a-z (lowercase)\n", letter);
        return 3;
    }

    if (ext_mp[letter-'a']) {
        printf("Error, a file system is already mounted at letter %c\n", letter);
        return 4;
    }

    partition* cpart = get_partition(drive, pnum);
    if (!cpart) {
        printf("Unable to mount filesystem with drive %d, and partition number %d\n", drive, pnum);
        return 1;
    }

    fs_type* fstype;
    if (cpart->type == 0x0B) {
        fstype = &fat32_type;
    } else {
        printf("Unknown filesystem type %x, aborting mount\n", cpart->type);
        return 2;
    }

    void* vbr = fstype->get_br(cpart->drive, cpart->sector);

    mountpoint* mp = kmalloc(sizeof(mountpoint));
    mp->type = fstype;
    mp->bootrecord = vbr;
    mp->drive = drive;
    mp->sector = cpart->sector;
    
    /*if (mp->type == &fat32_type) {
        show_info_fat32(vbr);
    }*/

    ext_mp[letter-'a'] = mp;
    return 0;
}

int unmount(char letter) {
    if (letter < 'a' || letter > 'z') {
        printf("Error, invalid drive letter %c, unable to unmount\n", letter);
        return 1;
    }

    mountpoint* mp = ext_mp[letter-'a'];
    ext_mp[letter-'a'] = 0;
    kfree(mp->bootrecord);
    kfree(mp);
    return 0;
}
