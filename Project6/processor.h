//
// Created by a on 1/11/17.
//

#ifndef PROJECT6_PROCESSOR_H
#define PROJECT6_PROCESSOR_H

#include <stdbool.h>
#include <stdint.h>

void processor_init();
void processor_add(int fd, bool in, void *aux);

typedef struct processor_state{
    int fd;
    long long (*start_routine) (struct processor_state *);
}processor_state;


#endif //PROJECT6_PROCESSOR_H
