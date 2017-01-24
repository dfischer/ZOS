#include <stdio.h>
#include <string.h>

#if defined(__is_libk)
#include <kernel/vmmngr.h>
#include <kernel/vfs.h>
#include <kernel/kmalloc.h>
#endif

FILE* fopen(const char* filename, const char* mode) {

#if defined(__is_libk)
    FILE* stream = kmalloc(sizeof(FILE));
    int fd = open(filename); // mode should go here, but for now we're omitting it..
    int block_size = get_block_size(fd); // Not used... for now...
    unsigned char* buffer = allocate_page();
#else
    // Otherwise we need to use the libc malloc (not yet implemented...)
#endif

    if (strcmp(mode, "r") == 0) {
        int result = read(fd, buffer);
    }
    
    // The current position is be default 0, i.e. the start of the file
    stream->buffer = buffer;
    stream->buffer_size = block_size;
    stream->file_descriptor = fd;

    return stream;
}
