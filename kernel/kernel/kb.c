#include <stdio.h>

#include <kernel/intex.h>
#include <kernel/kb.h>
#include <kernel/kmalloc.h>

unsigned char kbdus[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
        '9', '0', '-', '=', '\b',       /* Backspace */
        '\t',                   /* Tab */
        'q', 'w', 'e', 'r',     /* 19 */
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   /* Enter key */
        19,                     /* 29   - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',       /* 39 */
        '\'', '`',   21,                /* Left shift */
        '\\', 'z', 'x', 'c', 'v', 'b', 'n',                     /* 49 */
        'm', ',', '.', '/',   21,                               /* Right shift */
        '*',
        20,     /* Alt */
        ' ',    /* Space bar */
        18,     /* Caps lock */
        0,      /* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,      /* < ... F10 */
        17,     /* 69 - Num lock*/
        16,     /* Scroll Lock */
        0,      /* Home key */
        0,      /* Up Arrow */
        0,      /* Page Up */
        '-',
        0,      /* Left Arrow */
        0,
        0,      /* Right Arrow */
        '+',
        0,      /* 79 - End key*/
        0,      /* Down Arrow */
        0,      /* Page Down */
        0,      /* Insert Key */
        0,      /* Delete Key */
        0,   0,   0,
        0,      /* F11 Key */
        0,      /* F12 Key */
        0       /* All other keys are undefined */
};

unsigned char kbdus_shift[128] = {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
        '(', ')', '_', '+', '\b',       /* Backspace */
        '\t',                   /* Tab */
        'Q', 'W', 'E', 'R',     /* 19 */
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   /* Enter key */
        19,                     /* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'J', 'L', ':',       /* 39 */
        '\"', '~',   21,                /* Left shift */
        '|', 'Z', 'X', 'C', 'V', 'B', 'N',                      /* 49 */
        'M', '<', '>', '?',   21,                               /* Right shift */
        '*',
        20,     /* Alt */
        ' ',    /* Space bar */
        18,     /* Caps lock */
        0,      /* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,      /* < ... F10 */
        17,     /* 69 - Num lock*/
        16,     /* Scroll Lock */
        0,      /* Home key */
        0,      /* Up Arrow */
        0,      /* Page Up */
        '-',
        0,      /* Left Arrow */
        0,
        0,      /* Right Arrow */
        '+',
        0,      /* 79 - End key*/
        0,      /* Down Arrow */
        0,      /* Page Down */
        0,      /* Insert Key */
        0,      /* Delete Key */
        0,   0,   0,
        0,      /* F11 Key */
        0,      /* F12 Key */
        0       /* All other keys are undefined */
};

unsigned char keystatus = 0; /* x x x x x ctrl alt shift*/

char_handler* first_callback = 0;

void register_callback(handler_func func) {
    char_handler* current_callback;
    if (first_callback) {
        current_callback = first_callback;
        while (current_callback->next) {
            current_callback = current_callback->next;
        }
        current_callback->next = kmalloc(sizeof(char_handler));
        current_callback = current_callback->next;
    } else {
        first_callback = current_callback = kmalloc(sizeof(char_handler));
    }
    current_callback->func = func;
}

unsigned char unregister_callback(handler_func func) {
    char_handler* current_callback = first_callback;
    char_handler* last_callback = 0;
    while (current_callback) {
        if (current_callback->func == func) {
            if (last_callback) {
                last_callback->next = current_callback->next;
            } else {
                first_callback = 0;
            }
            kfree(current_callback);
            return 0;
        }

        last_callback = current_callback;
        current_callback = current_callback->next;
    }
    return 1;
}

void keyboard_handler(regs_t* r) {
	unsigned char scancode = inportb(0x60);
	
	unsigned char c;
	if (keystatus & 0x01) c = kbdus_shift[scancode];
	else c = kbdus[scancode];
	
    char_handler* current_callback;
	if (scancode & 0x80) { /* Key was released*/
		switch(c) {
		case 19: keystatus &= 0xFB; break;
		case 20: keystatus &= 0xFD; break;
		case 21: keystatus &= 0xFE; break;
		}
	} else { /* Key pressed*/
		switch(c) {
		case 19: keystatus |= 0x04; break;
		case 20: keystatus |= 0x02; break;
		case 21: keystatus |= 0x01; break;
		
		default: 
            //printf("%c", (char)c);
            current_callback = first_callback;
            while (current_callback) {
                current_callback->func(c);
                current_callback = current_callback->next;
            }
		}
	}
}

void kb_install() {
	irq_install_handler(1, keyboard_handler);
}
