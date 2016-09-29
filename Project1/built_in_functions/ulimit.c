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
    command_explained *cexOne = construct_command_explained_with_the_rest(cex);
    char * a = next_parameter_value(cexOne);
    ret = getrlimit(flag, r);
    if(is_number(a) == 1){
        limit = atoll(a);
        if(strcmp("H", next_parameter_value(cexOne)) == 0){
            r->rlim_max = limit;
        }else{
            r->rlim_cur = limit;
        }
        ret = setrlimit(flag, r);
        free(r);
        destruct_command_explained(cexOne);
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
            destruct_command_explained(cexOne);
            return ret;
        }
}
    

    void AFlag(command_explained *cex, struct rlimit *r){
        char * ret = malloc(20);
        char * info;

        gio_itoa(FlagUlimit(cex, r, RLIMIT_CORE), ret, 10);
        info = "\n core file size          (blocks, -c) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_DATA), ret, 10);
        info = "\n data seg size           (kbytes, -d) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_RTTIME), ret, 10);
        info = "\n scheduling priority             (-e) "; 
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_FSIZE), ret, 10);
        info = "\n file size               (blocks, -f) "; 
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_SIGPENDING), ret, 10);
        info = "\n pending signals                 (-i) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_MEMLOCK), ret, 10);
        info = "\n max locked memory       (kbytes, -l) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_RSS), ret, 10);
        info = "\n max memory size         (kbytes, -m) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));
         
        gio_itoa(FlagUlimit(cex, r, RLIMIT_NOFILE), ret, 10);
        info = "\n open files                      (-n) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_NOFILE), ret, 10);
        info = "\n pipe size            (512 bytes, -p) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));
    
        gio_itoa(FlagUlimit(cex, r, RLIMIT_MSGQUEUE), ret, 10);
        info = "\n POSIX message queues     (bytes, -q) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));
        
        gio_itoa(FlagUlimit(cex, r, RLIMIT_RTPRIO), ret, 10);
        info = "\n real-time priority              (-r) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_STACK), ret, 10);
        info = "\n stack size              (kbytes, -s) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_CPU), ret, 10);
        info = "\n cpu time               (seconds, -t) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));
        
        gio_itoa(FlagUlimit(cex, r, RLIMIT_NPROC), ret, 10);
        info = "\n max user processes              (-u) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_AS), ret, 10);
        info = "\n virtual memory          (kbytes, -v) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        gio_itoa(FlagUlimit(cex, r, RLIMIT_LOCKS), ret, 10);
        info = "\n file locks                      (-x) ";
        write(STDOUT_FILENO, info, strlen(info));
        write(STDOUT_FILENO, ret, strlen(ret));

        free(ret);
}


int MyUlimit(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    struct rlimit *r = malloc(2 * sizeof(rlim_t));
    while ((s = next_parameter_value(cex)) != NULL) {
        if(strcmp("-a", s) == 0){
            AFlag( cex, r);
            return 1;
        }else if(strcmp("-c", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_CORE);
        }else if(strcmp("-d", s) == 0){ 
                return FlagUlimit(cex, r, RLIMIT_DATA);
        }else if(strcmp("-e", s) == 0){ 
                return FlagUlimit(cex, r, RLIMIT_RTTIME);
        }else if(strcmp("-f", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_FSIZE);
        }else if(strcmp("-i", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_SIGPENDING);
        }else if(strcmp("-l", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_MEMLOCK);
        }else if(strcmp("-m", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_RSS);
        }else if(strcmp("-n", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_NOFILE); 
        }else if(strcmp("-p", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_NOFILE);
        }else if(strcmp("-q", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_MSGQUEUE);
        }else if(strcmp("-r", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_RTPRIO);
        }else if(strcmp("-s", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_STACK);
        }else if(strcmp("-t", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_CPU);
        }else if(strcmp("-u", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_NPROC);
        }else if(strcmp("-v", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_AS);
        }if(strcmp("-x", s) == 0){
                return FlagUlimit(cex, r, RLIMIT_LOCKS);
        }

   }



    return 0;
}

