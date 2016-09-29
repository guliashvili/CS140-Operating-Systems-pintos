//
// Created by Giorgi Guliashvili on 9/23/16.
//

#ifndef PROJECT1_CONTROLLER_H
#define PROJECT1_CONTROLLER_H

#include "parser.h"
int control_split_commands(char *line);

int get_last_child_return_code();

int control_command(command_explained *command, int *last_pipe, int *cur_pipe);
#endif //PROJECT1_CONTROLLER_H
