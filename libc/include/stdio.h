#ifndef _STDIO_H
#define _STDIO_H 1
 
#include <sys/cdefs.h>
 
#define EOF (-1)
 
#ifdef __cplusplus
extern "C" {
#endif

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


int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);

int fgetc(FILE* stream);

#ifdef __cplusplus
}
#endif
 
#endif
