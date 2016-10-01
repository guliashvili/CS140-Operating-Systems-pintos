#define _GNU_SOURCE
#include "ulimit.h"
#include "utility.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>


int FlagUlimit(command_explained *cex, struct rlimit *r, int flag) {
    long limit;
    int ret;
    command_explained *cexOne = construct_command_explained_with_the_rest(cex);
    char *a = next_parameter_value(cexOne);
    ret = getrlimit(flag, r);
    if (is_number(a) == 1) {
        limit = atoll(a);
        if(flag == RLIMIT_STACK) limit *= 1024;
        if (strcmp("H", next_parameter_value(cexOne)) == 0) {
            r->rlim_max = limit;
        } else {
            r->rlim_cur = limit;
        }
        ret = setrlimit(flag, r);
        free(r);
        destruct_command_explained(cexOne);
        return ret;
    } else {
        char *num = malloc(20);
        rlim_t to_act;
        if (a != NULL && strcmp("H", a) == 0) {
            to_act = r->rlim_max;
        } else {
            to_act = r->rlim_cur;
        }
        if(RLIMIT_STACK == flag) to_act /= 1024;
        if(to_act > 184467440737095){
            strcpy(num, "unlimited");
        }else {
            gio_itoa(to_act, num, 10);
        }
        write(STDOUT_FILENO, num, strlen(num));
        free(num);
        destruct_command_explained(cexOne);
        return ret;
    }
}

int pipe_buffer_size() {
    int ar[2];
    pipe(ar);
    long ret = fcntl(ar[1], F_GETPIPE_SZ);
    close(ar[1]);
    close(ar[0]);
    char* s = malloc(20);
    gio_itoa(ret / 8 / 1024, s, 10);
    write(STDOUT_FILENO, s, strlen(s));
    free(s);
    return 0;
}

void AFlag(command_explained *cex, struct rlimit *r) {
    char *info;


    info = "\n core file size          (blocks, -c) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_CORE);

    info = "\n data seg size           (kbytes, -d) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_DATA);

    info = "\n scheduling priority             (-e) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_RTTIME);

    info = "\n file size               (blocks, -f) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_FSIZE);

    info = "\n pending signals                 (-i) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_SIGPENDING);

    info = "\n max locked memory       (kbytes, -l) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_MEMLOCK);

    info = "\n max memory size         (kbytes, -m) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_RSS);

    info = "\n open files                      (-n) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_NOFILE);

    info = "\n pipe size            (512 bytes, -p) ";
    write(STDOUT_FILENO, info, strlen(info));
    pipe_buffer_size();

    info = "\n POSIX message queues     (bytes, -q) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_MSGQUEUE);

    info = "\n real-time priority              (-r) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_RTPRIO);

    info = "\n stack size              (kbytes, -s) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_STACK);

    info = "\n cpu time               (seconds, -t) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_CPU);

    info = "\n max user processes              (-u) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_NPROC);

    info = "\n virtual memory          (kbytes, -v) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_AS);

    info = "\n file locks                      (-x) ";
    write(STDOUT_FILENO, info, strlen(info));
    FlagUlimit(cex, r, RLIMIT_LOCKS);
}


int MyUlimit(command_explained *cex) {
    if (cex == NULL) return -1;
    char *s;
    struct rlimit *r = malloc(2 * sizeof(rlim_t));
    while ((s = next_parameter_value(cex)) != NULL) {
        if (strcmp("-a", s) == 0) {
            AFlag(cex, r);
            return 1;
        } else if (strcmp("-c", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_CORE);
        } else if (strcmp("-d", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_DATA);
        } else if (strcmp("-e", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_NICE);
        } else if (strcmp("-f", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_FSIZE);
        } else if (strcmp("-i", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_SIGPENDING);
        } else if (strcmp("-l", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_MEMLOCK);
        } else if (strcmp("-m", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_RSS);
        } else if (strcmp("-n", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_NOFILE);
        } else if (strcmp("-p", s) == 0) {
            return pipe_buffer_size();
        } else if (strcmp("-q", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_MSGQUEUE);
        } else if (strcmp("-r", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_RTPRIO);
        } else if (strcmp("-s", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_STACK);
        } else if (strcmp("-t", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_CPU);
        } else if (strcmp("-u", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_NPROC);
        } else if (strcmp("-v", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_AS);
        }
        if (strcmp("-x", s) == 0) {
            return FlagUlimit(cex, r, RLIMIT_LOCKS);
        }

    }


    return 0;
}