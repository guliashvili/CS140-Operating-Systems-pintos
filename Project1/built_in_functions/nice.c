#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <string.h>
#include "../parser.h"
#include <sys/wait.h>
#include "nice.h"
#include "utility.h"
#include "../controller.h"

int MyNice(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s = next_parameter_value(cex);
    int niceness;
    if (s == NULL) {
        niceness = nice(0);
        char *s = malloc(20);
        gio_itoa(niceness, s, 10);
        int ret = write(STDOUT_FILENO, s, strlen(s));

        free(s);
        return ret;
    } else {
        int increase_priority = 10;
        if (is_number(s + 1)) {
            increase_priority = atoi(s + 1);
        } else if (strcmp("--adjustment", s) == 0 || strcmp("-n", s) == 0) {
            s = next_parameter_value(cex);
            if (s == NULL) return -1;
            increase_priority = atoi(s);
        } else
            decrease_it(cex);

        command_explained *ce = construct_command_explained_with_the_rest(cex);


        nice(increase_priority);
        int a[2] = {-1, -1}, b[2] = {-1, -1};
        int ret = control_command(ce, a, b);
        destruct_command_explained(ce);
        return ret;


    }
}

