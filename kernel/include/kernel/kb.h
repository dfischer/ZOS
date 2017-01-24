#ifndef _KERNEL_KB_H
#define _KERNEL_KB_H

typedef void (*handler_func)(char c);

typedef struct char_handlerr {
    handler_func func;
    struct char_handlerr* next;
} char_handler;

void kb_install(void);
void register_callback(handler_func func);
unsigned char unregister_callback(handler_func func);

#endif
