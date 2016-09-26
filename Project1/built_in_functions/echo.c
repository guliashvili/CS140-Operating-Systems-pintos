#include "echo.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <stdio.h>

int echo(command_explained  * cex){
	if(cex == NULL) return -1;
	char *s;
	pid_t child;
	while((s = next_parameter_value(cex)) != NULL){
		if(s[0] =='$'){
			if*(s[1] !='?'){
				s = s + 1;
				char * val = getenv(s);
				return write(STDOUT_FILENO, val, strlen(val));
			}else{
				/////
			}
		}else{
            int k = 0;
            char* print = malloc(strlen(s));
            for(int i = 0; s[i]; i++)
                if(s[i] != '\"')
                    print[k++] = s[i];
            print[k] = 0;
            int ret = write(STDOUT_FILENO, print, strlen(print));
            free(print);
			return ret;
		}
	}

	return -1;
}
