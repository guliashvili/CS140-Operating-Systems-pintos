#include "echo.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int echo(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    int child;
    int first_line = 1;
    while ((s = next_parameter_value(cex)) != NULL) {
        if (s[0] == '$') {
            if (s[1] != '?') {
                s = s + 1;
                char *val = getenv(s);
                if (!first_line) write(STDOUT_FILENO, " ", 1);
                int ret = write(STDOUT_FILENO, val, strlen(val));
                if (ret < 0) return ret;
            } else {
                child = get_last_child_return_code();
                if (!first_line) write(STDOUT_FILENO, " ", 1);
                int ret = write(STDOUT_FILENO, &child, sizeof(int));
                if (ret < 0) return ret;
            }
        } else {
            if (!first_line) write(STDOUT_FILENO, " ", 1);
            int k = 0;
            for (int i = 0; s[i]; i++) if (s[i] != '\"') s[k++] = s[i];
            s[k] = 0;
            int ret = write(STDOUT_FILENO, s, strlen(s));
            if (ret < 0) return ret;
        }
        first_line = 0;
    }
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}
