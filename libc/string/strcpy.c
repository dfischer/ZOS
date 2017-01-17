#include <string.h>

char* strcpy(char* dstptr, const char* srcptr) {
    unsigned char* dst = (unsigned char*) dstptr;
    const unsigned char* src = (const unsigned char*) srcptr;

    unsigned char val = src[0];
    unsigned int i = 0;
    while (val) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
    return dstptr;
}
