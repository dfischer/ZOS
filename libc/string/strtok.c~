#include <string.h>
#include <stdbool.h>

char* olds;

char* strtok(char* s1, char* delim) {
    if (s1) { // This is the first call
        olds = s1;
    } else if (olds) { // The is a subsequent call
        s1 = olds;
    } else { // This is a subsequent call, but there are no more tokens to be found
        return 0;
    }

    char* token_start = 0;
    bool found_delim;
    for (unsigned int i = 0; s1[i] != '\0'; i++) {
        found_delim = false;
        for (unsigned int j = 0; delim[j] != '\0'; j++) {
            if (s1[i] == delim[j]) {
                found_delim = true;
                break;
            }
        }
        if (!found_delim) {
            token_start = s1+i;
            break;
        }
    }

    if (!token_start) return 0;

    unsigned int i;
    bool found_end = false;
    for (i = 0; token_start[i] != '\0'; i++) {
        for (unsigned int j = 0; delim[j] != '\0'; j++) {
            if (token_start[i] == delim[j]) {
                token_start[i] = '\0';
                found_end = true;
                break;
            }
        }
        if (found_end) break;
    }
    
    if (found_end) {
        olds = token_start+i+1;
    } else {
        olds = 0;
    }

    return token_start;
}
