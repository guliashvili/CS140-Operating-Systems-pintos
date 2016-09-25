//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include "controller.h"
#include "parser.h"
//#define NDEBUG
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
    int first = 1;
    while(1){
        char* s;
        if(first) s = readline("gioo: "),first=0;
        else s = readline("\ngioo: ");
        write_history(s);

        control_split_commands(s);

        free(s);
    }
#endif
    return 0;
}

