#include "ulimit.h"
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>


int MyUlimit(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    long limit;
    struct rlimit *r = malloc(2 * sizeof(rlim_t));
    while ((s = next_parameter_value(cex)) != NULL) {
        limit = atoll(next_parameter_value(cex));
        int get = getrlimit(RLIMIT_AS, r);
        if (strcmp("UL_GETFSIZE", s) == 0) {
            free(r);
            return get;

        } else {
            if (r->rlim_max < r->rlim_cur) return -1;
            r->rlim_cur = limit;
            free(r);
            return setrlimit(RLIMIT_AS, r);
        }

    }

    return 0;
}
	
	
