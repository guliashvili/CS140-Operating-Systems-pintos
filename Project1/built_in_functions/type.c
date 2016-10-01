#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "../parser.h"
#include <dirent.h>
#include <unistd.h>

static char *builtIns[] = {"?", "cd", "exit", "kill", "pwd", "ulimit", "nice", "echo", "type", "export", NULL};
static char **directories = NULL;

int containsStrr(char *str) {
    for (int i = 0; builtIns[i]; i++) {
        if (strcmp(builtIns[i], str) == 0) return 1;
    }
    return -1;
}

void getDirectories(char *allDir) {
    directories = malloc(sizeof(char*));
    int counter = 0;
    int last = 0;
    int size = strlen(allDir);
    for (int i = 0; i <= size; i++) {
        if (allDir[i] == 0 || allDir[i] == ':') {
            char *dest = malloc(i - last + 1);
            memcpy(dest, allDir + last, i - last);
            dest[i - last] = 0;
            directories[counter++] = dest;
            last = i + 1;

            directories = realloc(directories, sizeof(char*) * (counter + 1));
        }
    }
    directories[counter] = NULL;
}


void freeFnType(char *buff[]) {
    for (int i = 0; buff[i]; i++) {
        free(buff[i]);
        buff[i] = NULL;
    }
}

int type(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    while ((s = next_parameter_value(cex)) != NULL) {
        if (containsStrr(s)) {
            write(STDOUT_FILENO, s, strlen(s));
            char *tmp = " is a shell builtin\n";
            write(STDOUT_FILENO, tmp, strlen(tmp));
        }
        getDirectories(getenv("PATH"));
        DIR *dir;
        struct dirent *ent;
        //http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
        for (int i = 0; directories[i]; i++) {
            if ((dir = opendir(directories[i])) != NULL) {
                while ((ent = readdir(dir)) != NULL) {
                    if (strcmp(ent->d_name, s) == 0) {
                        write(STDOUT_FILENO, s, strlen(s));
                        write(STDOUT_FILENO, " is ", 4);
                        write(STDOUT_FILENO, directories[i], strlen(directories[i]));
                        write(STDOUT_FILENO, "/", 1);
                        write(STDOUT_FILENO, s, strlen(s));
                        write(STDOUT_FILENO, "\n", 1);
                    }
                }
                closedir(dir);
            }
        }
        freeFnType(directories);
        free(directories);
        return 0;
    }
    return -1;
}