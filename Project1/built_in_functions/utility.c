//
// Created by a on 9/28/16.
//

#include <ctype.h>
#include "utility.h"


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
        s[--len] = num % 10 + '0';
        num /= 10;
    } while (num);

}