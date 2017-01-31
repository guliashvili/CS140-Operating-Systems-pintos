//
// Created by a on 1/30/17.
//

#ifndef PROJECT6_CGI_BIN_H
#define PROJECT6_CGI_BIN_H

#include "map_entry.h"
#include "stdbool.h"

bool is_cgi_bin(const char *url);
int cgi_bin_execute(http_map_entry *root);

#endif //PROJECT6_CGI_BIN_H
