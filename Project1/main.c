//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include<signal.h>

#include "controller.h"
#include "parser.h"

#define NDEBUG
//#define TEST

void  func(int signal){

}

int main(int argc, char *argv[]) {
    signal(SIGINT,func);
    signal(SIGTSTP,func);
#ifdef TEST
    is_valid_line("./gio");
#else

#ifndef NDEBUG
    split_commands_info* commands_info =
            construct_split_commands("nice -a -b -c ./gio; chmod 'ax' && chmod 'bax' ");

    command_explained* command_info = construct_command_explained(commands_info->commands[0]);

    destruct_command_explained(command_info);
    destruct_split_commands(commands_info);

    test_is_valid_line();
    test_construct_command_explained();
#endif

    char *first_line = NULL;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 != argc) {
            first_line = argv[i + 1];
#ifndef NDEBUG
            int f = open("/home/a/Desktop/LOG.txt",  O_WRONLY | O_APPEND | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU);
            write(f, first_line, strlen(first_line));
            write(f, "\n", 1);
            close(f);
#endif
            return control_split_commands(first_line);
        }
    }
    read_history(NULL);
    int first = 1;
    while (1) {
        char *s;
        if (first) {
            s = readline("gioo: ");
            first = 0;
        } else
            s = readline("\ngioo: ");

        control_split_commands(s);

        add_history(s);
        write_history(NULL);
        free(s);
    }
#endif
    return 0;
}

