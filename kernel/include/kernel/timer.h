#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

int timer_ticks;

void timer_install(void);
void sleep(int);

#endif