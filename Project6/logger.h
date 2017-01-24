//
// Created by a on 1/23/17.
//

#ifndef PROJECT6_LOGGER_H
#define PROJECT6_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "stdbool.h"
#include <dirent.h>
#include "http_helper.h"
#include "config.h"
#include <sys/sendfile.h>
#include <fcntl.h>
#include <libut.h>
#include <sys/epoll.h>

struct log_info {
    struct in_addr ipAddr;
    int status_code;
    int sent_length;
    struct http_map_entry *root;
};

void log_write_info(struct log_info *log_info);

void log_write_error(struct log_info *log_info, const char *s);

#endif //PROJECT6_LOGGER_H
