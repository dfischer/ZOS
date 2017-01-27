#include <stdio.h>
#include <stdlib.h>
 
#if defined(__is_libk)
#include <kernel/tty.h>
#endif
 
int putchar(int ic) {
    char c = (char) ic;
#if defined(__is_libk)
    terminal_write(&c, sizeof(c));
#else
    syscall_terminal_write(&c, sizeof(c));
#endif
    return ic;
}
