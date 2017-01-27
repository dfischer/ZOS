#include <kernel/misc.h>

int hlt() {
    for(;;);
    return 0;
}
