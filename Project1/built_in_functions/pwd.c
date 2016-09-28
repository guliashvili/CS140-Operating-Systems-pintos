#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "pwd.h"

int pwd(command_explained *cex) {
    char *s = malloc(PATH_MAX + 1);
    char *curDir = getcwd(s, PATH_MAX + 1);
    int ret;
    if (curDir == NULL) ret = -1;
    else ret = write(STDOUT_FILENO, curDir, strlen(curDir));
    free(s);

    return ret;
}


