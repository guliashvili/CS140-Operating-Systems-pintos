//
// Created by Giorgi Guliashvili on 9/23/16.
//

#ifndef PROJECT1_PARSER_H
#define PROJECT1_PARSER_H

typedef enum command_linkage {AND, OR, ANYWAY} command_linkage;

typedef struct split_commands_info {
    char **commands;
    command_linkage *linkages;

    int commands_N;
}split_commands_info;

typedef struct command_explained {
    char* command;
    char** command_parameters;
    char* file_to_overwrite;
    char* file_to_read;
    char* file_to_append;

    int it;
}command_explained;

split_commands_info* construct_split_commands(const char * s);
void destruct_split_commands(split_commands_info* data);
void test_is_valid_line();

command_explained* construct_command_explained(const char* command);
command_explained* construct_command_explained_with_the_rest(command_explained* data);

void destruct_command_explained(command_explained* data);
void test_construct_command_explained();

char* next_parameter_value(command_explained* data);
int has_next_command(command_explained* data);

#endif //PROJECT1_PARSER_H
