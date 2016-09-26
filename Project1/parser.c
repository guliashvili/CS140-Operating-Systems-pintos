//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "parser.h"
#include <stdio.h>

#undef NULL
#define NULL ((void*)0)
void FREE(void* a){
    if(a != NULL){
        free(a);
    }
}

int is_valid_line(const char* s){
    if(s == NULL){
        return 0;
    }
    int andd = 0;
    int orr = 0;
    int semicolon = 0;
    int text = 0;
    int num_of_different;
    int sum = 0;

    int len = 0;
    int state = -1;

    for(; *s; s++){
        switch(*s){
            case '&':
                state = 0;
                andd++;
                break;
            case '|':
                state = 0;
                orr++;
                break;
            case ';':
                state = 0;
                semicolon++;
                break;
            default:
                state = 1;
                text++;
                break;
        }
        num_of_different = !!andd + !!orr + !!semicolon;
        sum = andd + orr + semicolon;
        if(state == 0) {
            if(sum == 1){
                if(text == 0) return -4;
                text = 0;
            }
            if (num_of_different > 1) {
                return -1;
            }else if(orr > 2) {
                return -6;
            }else if(andd > 2){
                return -7;
            }
        }else {
            if(text == 1){
                if(andd == 1){
                    return -8;
                }
                andd = orr = semicolon = sum = num_of_different = 0;
                len++;
            }
        }

    }
    if(sum) return -5;

    return len;

}


split_commands_info* construct_split_commands(const char * s){
    split_commands_info* ret = malloc(sizeof(split_commands_info));
    if(ret == NULL) return NULL;

    ret->commands = NULL;
    ret->commands_N = -1;
    ret->linkages = NULL;

    ret->commands_N = is_valid_line(s);
    if(ret->commands_N < 0){
        destruct_split_commands(ret);
        return NULL;
    }

    ret->commands = malloc(sizeof(char *) * ret->commands_N);
    if(ret->commands == NULL){
        destruct_split_commands(ret);
        return NULL;
    }

    ret->linkages = malloc(sizeof(command_linkage) * ((ret->commands_N == 0)? 0: (ret->commands_N - 1)));
    if(ret->linkages == NULL){
        destruct_split_commands(ret);
        return NULL;
    }

    for(int i = 0; i < ret->commands_N; i++){
        const char *find_pos = strpbrk(s, "&|;");
        if(!find_pos)
            find_pos = s + strlen(s);

        int len = find_pos - s;
        char *com = ret->commands[i] = malloc(sizeof(char) * (len + 1));
        if(com == NULL){
            destruct_split_commands(ret);
            return NULL;
        }
        com[len] = 0;
        strncpy(com, s, len);

        if(i + 1 != ret->commands_N){
            s = find_pos;
            if(*s == ';'){
                while(*s == ';') s++;
                ret->linkages[i] = ANYWAY;
            }else if(s[0] == '|' && s[1] == '|'){
                s += 2;
                ret->linkages[i] = OR;
            }else if(s[0] == '|'){
                s++;
                ret->linkages[i] = PIPE;
            }else if(s[0] == '&' && s[1] == '&'){
                s += 2;
                ret->linkages[i] = AND;
            }else{
                destruct_split_commands(ret);
                return NULL;
            }
        }
    }


    return ret;
}

void destruct_split_commands(split_commands_info* data){
    if(data == NULL) return;
    if(data->commands != NULL)
        for(int i = 0; i < data->commands_N; i++) FREE(data->commands[i]);
    FREE(data->commands);
    FREE(data->linkages);
    FREE(data);
}

char* my_strdup(const char* x){
    if(x == NULL) return NULL;
    int len = strlen(x);
    char* ret = malloc((len+1) * sizeof(char));
    for(int i = 0; x[i]; i++) ret[i] = x[i];
    ret[len] = 0;

    return ret;
}

command_explained* construct_command_explained(const char* commandd){
    command_explained* ret = malloc(sizeof(command_explained));
    if(ret == NULL) return NULL;

    ret->it = 0;
    ret->command_parameters = NULL;
    ret->command = NULL;
    ret->file_to_append = NULL;
    ret->file_to_overwrite = NULL;
    ret->file_to_read = NULL;

    char* command = NULL;
    {
        //remove more then one space, and spaces in the end or at the beginning
        char *s = my_strdup(commandd);
        if(s == NULL){
            destruct_command_explained(ret);
            return NULL;
        }
        int j = 0;
        int was_space = 1;
        for(int i = 0; s[i]; i++){
            if(s[i] != ' ' || (!was_space && j != 0)){
                was_space = s[i] == ' ';
                s[j++] = s[i];
            }
        }
        s[j] = 0;
        while(strlen(s) > 0 && s[strlen(s) - 1] == ' ') s[strlen(s) - 1] = 0;
        ret->command = s;
        command = s;
    }

    char* file_in = strpbrk(command, "<");
    char* file_out = strpbrk(command, ">");

    ret->file_to_append = ret->file_to_overwrite = ret->file_to_read = NULL;
    char *save = command;
    if(file_in != NULL){
        *file_in = 0;

        command = file_in + 1;
        if(*command == ' ') command++;

        const char* tmp = strpbrk(command, " ");
        if(tmp == NULL) tmp = command + strlen(command);

        file_in = strndup(command, tmp - command);
        if(file_in == NULL){
            destruct_command_explained(ret);
            return NULL;
        }

        ret->file_to_read = file_in;

    }
    if(file_out != NULL){
        *file_out = 0;

        command = file_out + 1;
        int is_append = 0;
        if(*command == '>'){
            is_append = 1;
            command++;
        }
        if(*command == ' ') command++;

        const char* tmp = strpbrk(command, " ");
        if(tmp == NULL) tmp = command + strlen(command);


        file_out = strndup(command, tmp - command);
        if(file_out == NULL){
            destruct_command_explained(ret);
            return NULL;
        }

        if(is_append) ret->file_to_append = file_out;
        else ret->file_to_overwrite = file_out;
    }
    command = save;
    while(strlen(command) > 0 && command[strlen(command) - 1] == ' ') command[strlen(command) - 1] = 0;

    save = command;
    int len = 0;
    if(command[0] == '.' && command[1] == '/'){
        command += 2;
        len++;
    }
    for(int i = 0; command[i]; i++) len += command[i] == ' ';
    len += strlen(command) > 0;
    len++;

    ret->command_parameters = malloc(sizeof(char *) * len);
    if(ret->command_parameters == NULL){
        destruct_command_explained(ret);
        return NULL;
    }


    command = save;

    int k = 0;
    if(command[0] == '.' && command[1] == '/') {
        command += 2;
        ret->command_parameters[k++] = strdup("./");
        if(ret->command_parameters == NULL){
            destruct_command_explained(ret);
            return NULL;
        }
    }
    while(*command) {
        if(*command == ' ') command++;
        const char *find_pos = strpbrk(command, " ");
        if(find_pos == NULL) find_pos = command + strlen(command);
        ret->command_parameters[k++] = strndup(command, find_pos - command);
        if(ret->command_parameters[k - 1] == NULL){
            destruct_command_explained(ret);
            return NULL;
        }

        command = (char*)find_pos;
    }
    ret->command_parameters[k++] = NULL;
    assert(k == len);


    return ret;
}

void destruct_command_explained(command_explained* data){

    if(data == NULL) return;
    FREE(data->file_to_append);

    FREE(data->file_to_overwrite);
    FREE(data->file_to_read);
    for(int i = 0; data->command_parameters[i]; i++){
        FREE(data->command_parameters[i]);
    }

    FREE(data->command_parameters);

    FREE(data->command);
    FREE(data);
}

int get_it(command_explained* data){
    if(data == NULL) return -1;
    return data->it;
}

void set_it(command_explained* data,int it){
    if(data == NULL) return;
    data->it = it;
}

void decrease_it(command_explained* data){
    if(data == NULL)
        return;
    data->it--;
}

char* next_parameter_value(command_explained* data){
    return data->command_parameters[data->it++];
}


command_explained* construct_command_explained_with_the_rest(command_explained* data) {
    if(data == NULL) return NULL;
    char *s;
    int len = 0;
    for(int i = data->it; data->command_parameters[i]; i++){
        len += strlen(data->command_parameters[i]) + 1;
    }
    s = malloc(sizeof(char) * len);
    int k = 0;
    for(int i = data->it; data->command_parameters[i]; i++){
        for(int j = 0; data->command_parameters[i][j];j++){
            s[k++] = data->command_parameters[i][j];
        }
        if(data->command_parameters[i+1]) s[k++] = ' ';
    }
    s[k++] = 0;

    command_explained* ret = construct_command_explained(s);
    FREE(s);
    return ret;
}

void test_is_valid_line(){
    assert(is_valid_line("a&a")<0);
    assert(is_valid_line("a|||a")<0);

    assert(is_valid_line("&&a")<0);
    assert(is_valid_line("||a")<0);
    assert(is_valid_line("|a")<0);


    assert(is_valid_line("a&&")<0);
    assert(is_valid_line("a||")<0);
    assert(is_valid_line("a|")<0);

    assert(is_valid_line("a>|")<0);

    assert(is_valid_line("qva|a||a&&a") == 4);
    assert(is_valid_line("qva|ax||bax&&cax;;;gg") == 5);
}


void test_construct_command_explained(){
    command_explained *a = construct_command_explained(" gio -c 5  -7 bax >>  ff.txt  ");
    assert(a != NULL);
    assert(strcmp(a->command, "gio -c 5 -7 bax") == 0);
    assert(strcmp(a->command_parameters[0], "gio") == 0);
    assert(strcmp(a->command_parameters[1], "-c") == 0);
    assert(strcmp(a->command_parameters[2], "5") == 0);
    assert(strcmp(a->command_parameters[3], "-7") == 0);
    assert(strcmp(a->command_parameters[4], "bax") == 0);
    assert(a->command_parameters[5] == NULL);

    assert(a->file_to_overwrite == NULL);
    assert(a->file_to_read == NULL);
    assert(strcmp(a->file_to_append, "ff.txt") == 0);
    destruct_command_explained(a);

    a = construct_command_explained(" ./gio -c 5  -7 bax >  ff.txt  ");
    assert(a != NULL);
    assert(strcmp(a->command, "./gio -c 5 -7 bax") == 0);
    assert(strcmp(a->command_parameters[0], "./") == 0);
    assert(strcmp(a->command_parameters[1], "gio") == 0);
    assert(strcmp(a->command_parameters[2], "-c") == 0);
    assert(strcmp(a->command_parameters[3], "5") == 0);
    assert(strcmp(a->command_parameters[4], "-7") == 0);
    assert(strcmp(a->command_parameters[5], "bax") == 0);
    assert(a->command_parameters[6] == NULL);

    assert(a->file_to_append == NULL);
    assert(strcmp(a->file_to_overwrite,"ff.txt") == 0);
    assert(a->file_to_read == NULL);

    destruct_command_explained(a);

    a = construct_command_explained(" ./gio -c 5  -7 bax <  ff.txt  ");
    assert(a != NULL);
    assert(strcmp(a->command, "./gio -c 5 -7 bax") == 0);
    assert(strcmp(a->command_parameters[0], "./") == 0);
    assert(strcmp(a->command_parameters[1], "gio") == 0);
    assert(strcmp(a->command_parameters[2], "-c") == 0);
    assert(strcmp(a->command_parameters[3], "5") == 0);
    assert(strcmp(a->command_parameters[4], "-7") == 0);
    assert(strcmp(a->command_parameters[5], "bax") == 0);
    assert(a->command_parameters[6] == NULL);
    assert(a->file_to_overwrite == NULL);
    assert(strcmp(a->file_to_read,"ff.txt") == 0);
    assert(a->file_to_append == NULL);

    assert(strcmp(next_parameter_value(a), "./") == 0);
    assert(strcmp(next_parameter_value(a), "gio") == 0);
    assert(strcmp(next_parameter_value(a), "-c") == 0);

    command_explained *x = construct_command_explained_with_the_rest(a);
    assert(x != NULL);

    assert(strcmp(x->command,"5 -7 bax") == 0);
    destruct_command_explained(x);
    destruct_command_explained(a);
}