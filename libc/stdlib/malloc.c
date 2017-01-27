#include <stdlib.h>

#if defined(__is_libk)
#include <kernel/vmmngr.h>
#include <kernel/kmalloc.h>
#endif

void* malloc(size_t size) {
#if defined(__is_libk)

#else
    return 0;
#endif
}
