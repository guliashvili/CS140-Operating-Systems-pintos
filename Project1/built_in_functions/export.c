#include "export.h"
#include <string.h>
#include <stdlib.h>

int export(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    char *varName;
    char *value;
    char *equalSign = "=";
    while ((s = next_parameter_value(cex)) != NULL) {
        value = strpbrk(s, equalSign) + 1;
        varName = malloc(value - s);
        strncpy(varName, s, value - s - 1);
        varName[value - s - 1] = 0;
        int result = setenv(varName, value, 1);
        free(varName);
        return result;
    }

    return -1;
}
