#include <stdint.h>
#include <stdio.h>

#include <kernel/syscall.h>
#include <kernel/intex.h>

#include <kernel/tty.h>
#include <kernel/misc.h>
#include <kernel/task.h>

uint32_t num_syscalls = 3;
static void *syscalls[3] =  // THESE FUNCTIONS HAVE TO RETURN SOMETHING!!!
{
   &terminal_write,
   &hlt,
   &kill_current_process,
};

void syscall_handler(regs_t *regs) {
   // Firstly, check if the requested syscall number is valid.
   // The syscall number is found in EAX.
   if (regs->eax >= num_syscalls) {
       printf("Error, syscall number %d is not defined\n", regs->eax);
       return;
   }

   //printf("syscall %d called\n", regs->eax);

   // Get the required syscall location.
   void *location = syscalls[regs->eax];

   // We don't know how many parameters the function wants, so we just
   // push them all onto the stack in the correct order. The function will
   // use all the parameters it wants, and we can pop them all back off afterwards.
   int ret = 0;
   __asm__ __volatile__ (" \
     pusha;\
     push %1; \
     push %2; \
     push %3; \
     push %4; \
     push %5; \
     call *%6; \
     add $0x14,%%esp; \
     popa;\
   " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
   regs->eax = ret;

}

void init_syscalls() {
   irq_install_handler(0x80, &syscall_handler);
}
