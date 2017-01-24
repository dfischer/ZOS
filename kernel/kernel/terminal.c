#include <stdio.h>
#include <string.h>

#include <kernel/kb.h>
#include <kernel/vfs.h>
#include <drivers/mbr.h>
#include <kernel/kmalloc.h>

const char* prompt = "$";
char cmd_buffer[1024];
unsigned int buffer_pos = 0;

fs_node* wd;

void run_cmd(char* command) {
    /*char* first_token = strtok(command, " ");
    if (!first_token) return;

    if (strcmp(first_token, "ls") == 0) {
        fat32_node* list = readdir_fat32(wd);
        while(list) {
            printf("%s    ", list->name);
            list = list->next;
        }
        printf("\n");
        free_nodelist(list);
    } else {
        printf("Error, command \"%s\" not understood\n", first_token);
    }*/
}

void c_typed(char c) {
    if (c == '\n') {
        printf("%c", c);
        char* command_str = kmalloc(buffer_pos+1);
        memcpy(command_str, cmd_buffer, buffer_pos);
        command_str[buffer_pos] = '\0';

        memset(cmd_buffer, 0, 1024); // We don't actually need this here...
        buffer_pos = 0;

        run_cmd(command_str);
        kfree(command_str);
        printf("%s ", prompt);
    } else if (c == '\b') {
        if (buffer_pos > 0) {
            printf("%c", c);
            cmd_buffer[--buffer_pos] = '\0';
        }
    } else {
        printf("%c", c);
        cmd_buffer[buffer_pos++] = c;
    }
}

void init_terminal(fs_node* rootnode) {
    printf("before getting rn\n");
    wd = rootnode;
    printf("aftering gettig rn\n");

    printf("%s ", prompt);
    register_callback(c_typed);
}
