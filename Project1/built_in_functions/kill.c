//
// Crea//
// Created by GIORGI GULIASHVILI on 9/23/16.
//
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "kill.h"


static char *wordArr[] = {"SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL",
                          "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD",
                          "SIGCONT",
                          "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM",
                          "SIGPROF", "SIGWINCH",
                          "SIGIO", "SIGPWR", "SIGSYS", "", "", "SIGRTMIN", "SIGRTMIN+1", "SIGRTMIN+2", "SIGRTMIN+3",
                          "SIGRTMIN+4", "SIGRTMIN+5",
                          "SIGRTMIN+6", "SIGRTMIN+7", "SIGRTMIN+8", "SIGRTMIN+9", "SIGRTMIN+10", "SIGRTMIN+11",
                          "SIGRTMIN+12", "SIGRTMIN+13",
                          "SIGRTMIN+14", "SIGRTMIN+15", "SIGRTMAX-14", "SIGRTMAX-13", "SIGRTMAX-12", "SIGRTMAX-11",
                          "SIGRTMAX-10", "SIGRTMAX-14",
                          "SIGRTMAX-13", "SIGRTMAX-12", "SIGRTMAX-11", "SIGRTMAX-10", "SIGRTMAX-9", "SIGRTMAX-8",
                          "SIGRTMAX-7", "SIGRTMAX-6", "SIGRTMAX-5",
                          "SIGRTMAX-4", "SIGRTMAX-3", "SIGRTMAX-2", "SIGRTMAX-1", "SIGRTMAX"};


int containsStr(char *str) {
    for (int i = 0; i < 64; i++) {
        if (i == 32 || i == 33) continue;
        if (strcmp(wordArr[i], str) == 0) return i;
        char *sbstr = malloc(strlen(wordArr[i]));
        strncpy(sbstr, wordArr[i] + 3, strlen(wordArr[i]) - 3);
        int ans = strcmp(sbstr, str) == 0;
        free(sbstr);
        if (ans) return i;
    }
    return -1;
}

void freeFn(char *buff[]) {
    for (int i = 0; i < 64; i++) {
        free(buff[i]);
    }
}

int is_number(char *s) {
    for (int i = 0; s[i]; i++) if (!isdigit(s[i])) return 0;
    return 1;
}

void gio_itoa(int num, char *s, int base) {
    int tmp = base;
    int len = 0;
    do {
        tmp /= 10;
        len++;
    } while (tmp);

    s[--len] = 0;

    do {
        s[--len] = num % 10;
        num /= 10;
    } while (num);

}

int kill1(command_explained *cex) {
    if (cex == NULL) return -1;

    char *s;
    int sig;
    int numArr[64];
    for (int i = 0; i < 64; i++) {
        if (i == 32 || i == 33) {
            numArr[i] = -1;
            continue;
        }
        numArr[i] = i;
    }

    while ((s = next_parameter_value(cex)) != NULL) {
        if (strcmp(s, "-l") == 0) {
            char *c = next_parameter_value(cex);
            if (c == NULL) { // მთლიანი სიის დაბეჭდვა
                char *buff[64];
                for (int i = 0; i < 64; i++) {
                    if (i == 32 || i == 33) {
                        continue;
                    } else {
                        char *dest = malloc(30);
                        char *strVal = wordArr[i];
                        char intVal[33];
                        gio_itoa(numArr[i], intVal, 10);
                        char *tmp = ") ";
                        strcat(dest, intVal);
                        strcat(dest, tmp);
                        buff[i] = strcat(dest, strVal);
                    }
                }
                write(STDOUT_FILENO, buff, 64);
                freeFn(buff);
                return 0;
            }
            int flag = 0;
            for (int i = 0; i < strlen(c); i++) {
                if (!isdigit(c + i)) {
                    flag = 1;
                    break;
                }
            }
            if (flag) { // რიცხვის დაბეჭდვა
                int contains = containsStr(c);
                if (contains >= 0) {
                    int res = numArr[contains];
                    char str[33];
                    gio_itoa(res, str, 10);
                    write(STDOUT_FILENO, str, strlen(str));
                } else {
                    //error
                }
            } else {
                int num = atoi(c);
                if (num < 1 || num > 64 || num == 32 || num == 33) {
                    //error
                } else {
                    char *res = wordArr[num] + 3;
                    write(STDOUT_FILENO, res, strlen(res));
                }
            }
        } else if (is_number(s + 1)) {
            sig = atoi(s + 1);
            char *pid = next_parameter_value(cex);
            if (pid == NULL)
                return -1; //error
            pid_t pid1 = atoi(pid);


            int ret = kill(pid1, sig);
            return ret;
        }
    }
    return -1;
}

