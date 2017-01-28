#include <stdio.h>

#if defined(__is_libk)
#include <kernel/vmmngr.h>
#include <kernel/kmalloc.h>
#else
// We need to include the libc malloc, normally
#endif

int fclose(FILE* stream) {
    // We also need to flush the buffer here!
#if defined(__is_libk)
    kfree(stream->buffer);
    close(stream->file_descriptor);
#else
    // otherwise we need to use the libc free (not yet implemented...)
#endif
    kfree(stream);
    return 0;
}
