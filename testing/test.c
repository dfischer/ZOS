#include <stdio.h>

int main() {
    unsigned char flags = 1;
      if (1) flags |= 1 << 1;
      if (1) flags |= 1 << 2;
      printf("flags: %x\n", flags);

    return 0;
}
