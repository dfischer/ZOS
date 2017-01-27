#ifndef _STDIO_H
#define _STDIO_H 1
 
#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>
 
#define EOF (-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct fpos {
    unsigned int clusternum;
    unsigned int clusteroffset;
} fpos_t;

typedef struct FILEE {
    fpos_t curpos;
    unsigned char* buffer;
    int buffer_size;
    int file_descriptor;
} FILE;

#ifdef __cplusplus
extern "C" {
#endif

FILE* stderr;
#define stderr stderr

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);

int fgetc(FILE* stream);
int fseek(FILE*, long, int);
size_t fread(void*, size_t, size_t, FILE*);

// These still need actual implementations:
int fflush(FILE* stream);
int fprintf(FILE*, const char*, ...);
long ftell(FILE*);
size_t fwrite(const void*, size_t, size_t, FILE*);
void setbuf(FILE*, char*);
int vfprintf(FILE*, const char*, va_list);

#ifdef __cplusplus
}
#endif
 
#endif
