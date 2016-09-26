//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include "controller.h"
#include "parser.h"

#include <unistd.h>
#include <fcntl.h>
#define NDEBUG
//#define TEST

int main(int argc, char* argv[]){
#ifdef TEST
is_valid_line("./gio");
#else

#ifndef NDEBUG
    for(int i = 0; i < argc; i++) puts(argv[i]);
    split_commands_info* commands_info =
            construct_split_commands("nice -a -b -c ./gio; chmod 'ax' && chmod 'bax' ");

    command_explained* command_info = construct_command_explained(commands_info->commands[0]);

    destruct_command_explained(command_info);
    destruct_split_commands(commands_info);

    test_is_valid_line();
    test_construct_command_explained();
#endif

    char* first_line = NULL;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-c") == 0 && i + 1 != argc){
            first_line = argv[i + 1];
#ifndef NDEBUG
            write(open("/home/a/Desktop/LOG.txt",  O_WRONLY | O_TRUNC | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU), first_line, strlen(first_line));
#endif
            return control_split_commands(first_line);
        }
    }
    read_history(NULL);
    int first = 1;
    fprintf(stderr, "gio");
    while(1){
        char* s;
        if(first){
            s = readline("gioo: ");
            first=0;
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

