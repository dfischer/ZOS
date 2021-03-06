#ifndef _STDLIB_H
#define _STDLIB_H 1
 
#include <sys/cdefs.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
 

__attribute__((__noreturn__))
void abort(void);
char* itoa(int, char*, int);
char* uitoa(unsigned int, char*, int);

int syscall_terminal_write(const char* data, size_t size);
int syscall_hlt();
int syscall_kill_current_process(int status);

// To be implemented... just declarations at the moment
int atexit(void (*)(void));
int atoi(const char*);
void free(void*);
char* getenv(const char*);
void* malloc(size_t);
void exit(int status); // This is actually implemented as a stub, but we need the full implementation!
 
#ifdef __cplusplus
}
#endif
 
#endif
