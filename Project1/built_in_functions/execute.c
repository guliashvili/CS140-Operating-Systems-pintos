
#include <unistd.h>
#include "execute.h"

int execute(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s = next_parameter_value(cex);
    if (s == NULL) return -1;
    decrease_it(cex);
    return execv(s, cex->command_parameters);
}


