#include <stdlib.h>

char* itoa(int value, char *str, int base) {
    char *res;
    char *ptr;
    char *low;
  
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    res = ptr = str;
  
    if (value < 0 && base == 10) {
        *ptr++ = '-';
    } else if (base == 16) {
        *ptr++ = '0';
        *ptr++ = 'x';
    }
    low = ptr;
    do {
        *ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + (value % base)];
        value /= base;
    } while (value);

    *ptr-- = '\0';
    while (ptr > low) {
        char temp = *low;
        *low++ = *ptr;
        *ptr-- = temp;
    }
    return res;
}

char* uitoa(unsigned int value, char *str, int base) {
    char *res;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    res = ptr = str;

    if (value < 0 && base == 10) {
        *ptr++ = '-';
    } else if (base == 16) {
        *ptr++ = '0';
        *ptr++ = 'x';
    }
    low = ptr;
    do {
        *ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + (value % base)];
        value /= base;
    } while (value);

    *ptr-- = '\0';
    while (ptr > low) {
        char temp = *low;
        *low++ = *ptr;
        *ptr-- = temp;
    }
    return res;
}
