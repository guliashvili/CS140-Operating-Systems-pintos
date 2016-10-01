#include <string.h>
#include "about.h"
#include <unistd.h>

int about(command_explained *cex) {
    char *aboutFSH = "This is Free Shell, you can use it like bash, shell or like some other command prompt";
    return write(STDOUT_FILENO, aboutFSH, strlen(aboutFSH));
}