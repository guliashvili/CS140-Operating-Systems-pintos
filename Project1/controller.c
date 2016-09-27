//
// Created by Giorgi Guliashvili on 9/23/16.
//

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "built_in_functions/cd.h"
#include "built_in_functions/pwd.h"
#include "built_in_functions/echo.h"
#include "built_in_functions/export.h"
#include "built_in_functions/execute_path.h"
#include "built_in_functions/exit.h"
#include "built_in_functions/execute.h"
#include "built_in_functions/kill.h"
#include "built_in_functions/ulimit.h"

static const char *command_names[] = {"./",
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
                                      "execute_from_PATH",
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
        0,
        1,
        -1
};

int (*functions[])(command_explained *command) = {
        execute,
        NULL,
        cd,
        pwd,
        MyExit,
        MyUlimit,
        NULL,
        kill1,
        NULL,
        echo,
        export,
        execute_path
};

static int last_child_status_code = 0;

int get_last_child_return_code() {
    return last_child_status_code;
}

int find_command_name_index(char *name) {
    int i = 0;
    for (i = 0; command_names[i]; i++) {
        if (strcmp(command_names[i], name) == 0) {
            return i;
        }
    }
    return i - 1;
}

int needs_fork(command_explained *command) {
    char *name = next_parameter_value(command);
    decrease_it(command);
    int i = find_command_name_index(name);
    return needs_fork_ar[i];
}

#include <stdio.h>
int control_command(command_explained *command, int *last_pipe, int *cur_pipe) {
    if (command == NULL)
        return -1;
    pid_t fork_id;
    if (needs_fork(command)) fork_id = fork();
    else fork_id = 0;

    if (fork_id == -1) {
        return -1;
    } else if (fork_id == 0) {
        int ret;
        int in = last_pipe[0];
        int out = cur_pipe[1];

        if (command->file_to_append != NULL) {
            out = open(command->file_to_append, O_WRONLY | O_APPEND | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU);
            if (out == -1) return -1;
        }
        if (command->file_to_overwrite != NULL) {
            out = open(command->file_to_overwrite, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
            if (out == -1) return -1;
        }
        if (command->file_to_read != NULL) {
            in = open(command->file_to_read, O_RDONLY);
            if (in == -1) return -1;
        }

        int in_save = -1, out_save = -1;
        if (in != -1) in_save = dup(STDIN_FILENO), dup2(in, STDIN_FILENO);
        if (out != -1) out_save = dup(STDOUT_FILENO), dup2(out, STDOUT_FILENO);
        //fprintf(stderr, "%d %d\n", in, out);

        char *name = next_parameter_value(command);
        if (name == NULL) assert(0);

        int commad_name_index = find_command_name_index(name);
        if (strcmp(command_names[commad_name_index], name) != 0) decrease_it(command);
        //fprintf(stderr, "hop");
        ret = functions[commad_name_index](command);
        //fprintf(stderr, "hop");

        //fprintf(stderr, "%d\n", in_save);
        if (in != -1) {
            //fprintf(stderr, "%d\n", in_save);
            close(STDIN_FILENO);
            dup2(in_save, STDIN_FILENO);
        }
        if (command->file_to_read != NULL) {
            close(in);
        }

        if (out != -1) {
            close(STDOUT_FILENO);
            dup2(out_save, STDOUT_FILENO);
        }
        if (command->file_to_append != NULL || command->file_to_overwrite != NULL) {
            close(out);
        }


        return ret;
    } else {
        int status;
        if (wait(&status))
            return last_child_status_code = WEXITSTATUS(status);
        else
            return -1;
    }
}


int control_split_commands(char *line) {
    split_commands_info *split_commands = construct_split_commands(line);
    if (split_commands == NULL) return -1;

    command_explained **commands_explained =
            malloc(sizeof(command_explained *) * split_commands->commands_N);
    for (int i = 0; i < split_commands->commands_N; i++) {
        commands_explained[i] = construct_command_explained(split_commands->commands[i]);
    }

    int ret = 0;

    int last_pipe[2] = {-1, -1};
    int cur_pipe[2] = {-1, -1};

    for (int i = 0; i < split_commands->commands_N; i++) {
        if (i != 0) {
            if (ret && split_commands->linkages[i - 1] == AND) continue;
            if (!ret && split_commands->linkages[i - 1] == OR) continue;
        }

        last_pipe[0] = cur_pipe[0];
        last_pipe[1] = cur_pipe[1];

        cur_pipe[0] = cur_pipe[1] = -1;

        if (i + 1 != split_commands->commands_N) {
            if (split_commands->linkages[i] == PIPE)
                pipe(cur_pipe);
        }

        ret = control_command(commands_explained[i], last_pipe, cur_pipe);

        if(cur_pipe[1] != -1){
            int ret = close(cur_pipe[1]);
            if(ret < 0) return ret;
            cur_pipe[1] = -1;
        }

        assert(last_pipe[1] == -1);

        if(last_pipe[0] != -1){
            int ret = close(last_pipe[0]);
            if(ret < 0) return ret;
            last_pipe[0] = -1;
        }
    }

    for (int i = 0; i < split_commands->commands_N; i++) {
        destruct_command_explained(commands_explained[i]);
    }

    free(commands_explained);

    destruct_split_commands(split_commands);

    return ret;
}

