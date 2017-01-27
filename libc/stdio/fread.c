#include <stdio.h>
#include <string.h>

#if defined(__is_libk)
#include <kernel/vfs.h>
#endif

size_t fread (void *data, size_t size, size_t count, FILE *stream) {
#if defined(__is_libk)
    if (stream->curpos.clusteroffset > stream->buffer_size) {
        printf("Error, the cluster offset is larger than the buffer size. This should never happen. aborting fgetc.\n");
        return 0;
    }

    if (size > stream->buffer_size) {
        printf("Error, reading a size greater than the buffer size is not currently supported. Read in smaller chunks for now.\n");
        return 0;
    }

    size_t i;
    for (i = 0; i < count; i++) {
        if (stream->curpos.clusteroffset+size > stream->buffer_size) {
            int num_prev_buffer = stream->buffer_size - stream->curpos.clusteroffset;
            memcpy((void*)((unsigned int)data+i*size), (void*)((unsigned int)(stream->buffer)+stream->curpos.clusteroffset), num_prev_buffer);


            stream->curpos.clusteroffset += num_prev_buffer;
            int result = read(stream->file_descriptor, stream->buffer);
            if (result) { // Some error reading, call it quits
                return i;
            }
            stream->curpos.clusteroffset = 0;
            stream->curpos.clusternum++;

            int num_to_read = size-num_prev_buffer;
            memcpy((void*)((unsigned int)data+i*size+num_prev_buffer), (void*)((unsigned int)(stream->buffer)), num_to_read);
            stream->curpos.clusteroffset += num_to_read;
        } else {
            memcpy((void*)((unsigned int)data+i*size), (void*)((unsigned int)(stream->buffer)+stream->curpos.clusteroffset), size);
            stream->curpos.clusteroffset += size;
        }
    }

    return i;
#else
    printf("Error, not implemented in user mode yet.\n");
    return 0;
#endif
}
