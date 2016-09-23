//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <stdlib.h>
#include <unistd.h>
#include "controller.h"
#include "parser.h"
int control_command(char *command_s){
    command_explained* command = construct_command_explained(command_s);


    destruct_command_explained(command);
    return 0;
}

int control_split_commands(char* line){
    split_commands_info *split_commands = construct_split_commands(line);
    if(split_commands == NULL) return -1;

    int ret;
    for(int i = 0; i < split_commands->commands_N; i++){
        ret = control_command(split_commands->commands[i]);
        if(i + 1 == split_commands->commands_N)
            continue;

    }
    destruct_split_commands(split_commands);
    return ret;
}

