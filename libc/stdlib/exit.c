#include <stdlib.h>

void exit(int status) {
    syscall_kill_current_process(status);
}
