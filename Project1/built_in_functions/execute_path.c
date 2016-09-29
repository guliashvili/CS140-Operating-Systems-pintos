
#include <unistd.h>
#include "execute_path.h"
int execute_path(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s = next_parameter_value(cex);
    if (s == NULL) return -1;
    decrease_it(cex);
    int ret =  execvp(s, cex->command_parameters);

    return ret;
}


