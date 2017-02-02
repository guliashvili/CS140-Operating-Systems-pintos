//
// Created by a on 1/11/17.
//

#ifndef PROJECT6_PROCESSOR_H
#define PROJECT6_PROCESSOR_H

#include <stdbool.h>
#include <stdint.h>
#include "logger.h"
/*
 * creates epoll and worker threads
 */
void processor_init();
/*
 * adds new socket in epoll
 */
void processor_add(int fd, bool in, void *aux);

typedef struct processor_state {
    int fd;
    int port;

    long long (*start_routine)(struct processor_state *);

    struct log_info log_data;
} processor_state;


#endif //PROJECT6_PROCESSOR_H
