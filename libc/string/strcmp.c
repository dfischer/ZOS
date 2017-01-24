#include <string.h>

int strcmp(const char* s1, const char* s2) {
    if (strlen(s1) > strlen(s2)) {
        int res = memcmp(s1, s2, strlen(s2));
        if (res) return res;
        return 1; // Because in this case we want s2 to come before s1
    } else if (strlen(s1) < strlen(s2)) {
        int res = memcmp(s1, s2, strlen(s1));
        if (res) return res;
        return -1; // Because in this case we want s1 to come before s2
    } else {
        return memcmp(s1, s2, strlen(s1));
    }
}
