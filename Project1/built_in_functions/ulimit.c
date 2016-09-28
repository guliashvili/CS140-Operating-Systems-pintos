#include "ulimit.h"
#include "utility.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



int FlagUlimit(command_explained *cex, struct rlimit *r, int flag){
    long limit;
    int ret;
    char * a = next_parameter_value(cex);
    ret = getrlimit(flag, r);
    if(is_number(a) == 1){
        limit = atoll(a);
        if(strcmp("H", next_parameter_value(cex)) == 0){
            r->rlim_max = limit;
        }else{
            r->rlim_cur = limit;
        }
        ret = setrlimit(flag, r);
        //free(r);
        return ret;
        }else{
            char * num = malloc(20);
            if(a != NULL && strcmp("H", a) == 0){
                gio_itoa(r->rlim_max, num, 10);
            }else{
                gio_itoa(r->rlim_cur, num, 10);
            }
            write(STDOUT_FILENO, num, strlen(num));
            free(num);
            return ret;
        }
}

int MyUlimit(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    struct rlimit *r = malloc(2 * sizeof(rlim_t));
    while ((s = next_parameter_value(cex)) != NULL) {
        if(strcmp("-a", s) == 0){
            return -1;//FlagUlimit(cex, r, 1111111111111); dasaweria
        }else if(strcmp("-c", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_CORE);
        }else if(strcmp("-d", s) == 0){ 
                return FlagUlimit(cex, r, RLIMIT_DATA);
        }else if(strcmp("-f", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_FSIZE);
        }else if(strcmp("-l", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_LOCKS);
        }else if(strcmp("-m", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_RSS);
        }else if(strcmp("-n", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_NOFILE);
        }else if(strcmp("-p", s) == 0){
                return -1;//FlagUlimit(cex, r, 1111111111111); dasaweria
        }else if(strcmp("-s", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_STACK);
        }else if(strcmp("-t", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_CPU);
        }else if(strcmp("-u", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_NPROC);
        }else if(strcmp("-v", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_AS);
        }

   }



    return 0;
}
	

