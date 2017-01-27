#include <stdlib.h>

int syscall_terminal_write(const char* data, size_t size) {
    int a;
    __asm__ __volatile__("int $0x80" : "=a" (a) : "0" (0), "b" (data), "c" (size));
    return a;
}

int syscall_hlt() {
    int a;
    __asm__ __volatile__("int $0x80" : "=a" (a) : "0" (1));
    return a;
}
