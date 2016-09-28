#include <string.h>
#include <unistd.h>
#include "cd.h"

int cd(command_explained *cex) {
    if (cex == NULL) return -1;
    char *notFollow = "-P";
    char *follow = "-L";
    char *s;
    while ((s = next_parameter_value(cex)) != NULL) {
        if (strcmp(notFollow, s) == 0) {
            // symbolic link ar vici ra aris
        } else if (strcmp(follow, s) == 0) {

        } else {
            return chdir(s);
        }
    }

    return -1;
}


