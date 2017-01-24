#include <stdio.h>

#if defined(__is_libk)
#include <kernel/vfs.h>
#endif


int fgetc(FILE* stream) {
    if (stream->curpos.clusteroffset > stream->buffer_size) {
        printf("Error, the cluster offset is larger than the buffer size. This should never happen. aborting fgetc.\n");
        return 0;
    }
    if (stream->curpos.clusteroffset == stream->buffer_size) {
        int result = read(stream->file_descriptor, stream->buffer);

        if (result == 2) return EOF;

        stream->curpos.clusteroffset = 0;
        stream->curpos.clusternum++;
    }
    return (int)(stream->buffer[stream->curpos.clusteroffset++]);
}
