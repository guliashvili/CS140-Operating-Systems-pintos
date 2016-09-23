//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <stdio.h>
#include "parser.h"

int main(int argc, char* argv[]){
    for(int i = 0; i < argc; i++) puts(argv[i]);
    split_commands_info* commands_info =
            construct_split_commands("nice -a -b -c ./gio; chmod 'ax' && chmod 'bax' ");

    command_explained* command_info = construct_command_explained(commands_info->commands[0]);

    destruct_command_explained(command_info);
    destruct_split_commands(commands_info);

    return 0;
}
