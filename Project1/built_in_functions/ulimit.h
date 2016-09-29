#ifndef PROJECT1_MYULIMIT_H
#define PROJECT1_MYULIMIT_H

#include "../parser.h"
#include <sys/resource.h>


int FlagUlimit(command_explained *cex, struct rlimit *r, int flag);

void AFlag(command_explained *cex, struct rlimit *r);

int MyUlimit(command_explained *cex);


#endif //PROJECT1_MYULIMIT_H
