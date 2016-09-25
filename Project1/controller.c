//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include "controller.h"
#include "parser.h"

static const char* command_names[] = {"./",
                                      "?",
                                      "cd",
                                      "pwd",
                                      "exit",
                                      "ulimit",
                                      "nice",
                                      "kill",
                                      "type",
                                      "echo",
                                      "export",
                                      NULL
};
static const int needs_fork_ar[] = {
        1,
        0,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0
        -1
};
#include<stdio.h>
int print(command_explained* command){
    puts(next_parameter_value(command));
    return 0;
}

int (*functions[]) (command_explained* command) = {
        print
};

int find_command_name_index(char* name){
    int i = 0;
    for(i = 0; command_names[i]; i++){
        if(strcmp(command_names[i], name) == 0){
            return i;
        }
    }
    return 0;
}
int needs_fork(command_explained* command){
    char * name = next_parameter_value(command);
    decrease_it(command);
    int i = find_command_name_index(name);
    return needs_fork_ar[i];
}
#define NO_FORK_NO_FORK 0
#define NO_FORK_FORK 1
#define FORK_NO_FORK 2
#define FORK_FORK 3

static int first_thread_close[4][2] = {
        {-1, 1},
        {-1, 1},
        {0, 1},
        {0, 1}
};
static int second_thread_close[4][2] = {
        {-1, 0},
        {-1, 0},
        {-1, 0},
        {-1, 0}
};
static int main_thread_close[4][3] = {
        {-1, -1, -1},
        {-1, -1, -1},
        {-1, 1, -1},
        {-1, 1, 0}
};
int control_command(command_explained* command, int* last_pipe, int last_pipe_mask, int* cur_pipe, int cur_pipe_mask){
    if(command == NULL)
        return -1;
    pid_t fork_id;
    if(needs_fork(command)) fork_id = fork();
    else fork_id = 0;

    if(fork_id == -1){
        return -1;
    }else if(fork_id == 0) {
        int ret;
        int in = last_pipe[0];
        int out = cur_pipe[1];

        int in_save = -1, out_save = -1;
        if(in != -1) in_save = dup(STDIN_FILENO),dup2(in, STDIN_FILENO);
        if(out != -1) out_save = dup(STDOUT_FILENO), dup2(out, STDOUT_FILENO);

        int cur_close_f = first_thread_close[cur_pipe_mask][0];
        int last_close_f = second_thread_close[last_pipe_mask][0];

        if (cur_close_f != -1)
            if(cur_pipe[cur_close_f] != -1){
                if (close((cur_pipe[cur_close_f]))) return -1;
                cur_pipe[cur_close_f] = -1;
            }
        if (last_close_f != -1)
            if(last_pipe[last_close_f] != -1) {
                if (close(last_pipe[last_close_f])) return -1;
                last_pipe[last_close_f] = -1;
            }


        char *name = next_parameter_value(command);
        if (name == NULL) assert(0);

        int commad_name_index = find_command_name_index(name);

        ret = functions[commad_name_index](command);

        int cur_close_l = first_thread_close[cur_pipe_mask][1];
        int last_close_l = second_thread_close[last_pipe_mask][1];

        if (cur_close_l != -1)
            if(cur_pipe[cur_close_l] != -1) {
                if (close(cur_pipe[cur_close_l])) return -1;
                cur_pipe[cur_close_l] = -1;
            }
        if (last_close_l != -1)
            if(last_pipe[last_close_l] != -1) {
                if (close(last_pipe[last_close_l])) return -1;
                last_pipe[last_close_l] = -1;
            }

        if(in != -1){
            close(STDIN_FILENO);
            dup2(in_save, STDIN_FILENO);
        }

        if(out != -1){
            close(STDOUT_FILENO);
            dup2(out_save, STDOUT_FILENO);
        }


        return  ret;
    }else{
        int status;
        if(wait(&status))
            return WEXITSTATUS(status);
        else
            return -1;
    }
}



int control_split_commands(char* line){
    split_commands_info *split_commands = construct_split_commands(line);
    if(split_commands == NULL) return -1;

    command_explained** commands_explained =
            malloc(sizeof(command_explained*) * split_commands->commands_N);
    for(int i = 0; i < split_commands->commands_N; i++) {
        commands_explained[i] = construct_command_explained(split_commands->commands[i]);
    }

    int ret = 0;

    int last_pipe[2] = {-1, -1};
    int last_pipe_mask = 0;

    int cur_pipe[2] = {-1, -1};
    int cur_pipe_mask = 0;

    for(int i = 0; i < split_commands->commands_N; i++){
        if(i != 0){
            if(ret && split_commands->linkages[i - 1] == AND) continue;
            if(!ret && split_commands->linkages[i - 1] == OR) continue;
        }

        last_pipe_mask = cur_pipe_mask;
        last_pipe[0] = cur_pipe[0];
        last_pipe[1] = cur_pipe[1];

        cur_pipe_mask = 0;
        cur_pipe[0] = cur_pipe[1] = -1;

        if(i + 1 != split_commands->commands_N){
            if(split_commands->linkages[i] == PIPE){
                pipe(cur_pipe);
                if(needs_fork(commands_explained[i])) cur_pipe_mask |= 2;
                if(needs_fork(commands_explained[i + 1])) cur_pipe_mask |= 1;
            }
        }

        int cur_command_f = main_thread_close[cur_pipe_mask][0];
        int last_command_m = main_thread_close[last_pipe_mask][1];
        if(cur_command_f != -1){
            if(cur_pipe[cur_command_f] != -1) {
                if (close(cur_pipe[cur_command_f])) return -1;
                cur_pipe[cur_command_f] = -1;
            }
        }
        if(last_command_m != -1){
            if(last_pipe[last_command_m] != -1) {
                if (close(last_pipe[last_command_m])) return -1;
                last_pipe[last_command_m] = -1;
            }
        }

        ret = control_command(commands_explained[i], last_pipe, last_pipe_mask, cur_pipe, cur_pipe_mask);

        int last_command_e = main_thread_close[last_pipe_mask][2];
        if(last_command_e != -1){
            if(last_pipe[last_command_e] != -1){
                if(close(last_command_e)) return -1;
                last_pipe[last_command_e] = -1;
            }

        }

    }

    for(int i = 0; i < split_commands->commands_N; i++) {
        destruct_command_explained(commands_explained[i]);
    }

    free(commands_explained);

    destruct_split_commands(split_commands);

    return ret;
}

