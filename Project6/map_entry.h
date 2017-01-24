//
// Created by a on 1/11/17.
//

#ifndef PROJECT6_MAP_ENTRY_H
#define PROJECT6_MAP_ENTRY_H

#include "uthash.h"

typedef struct config_map_entry {
    char *key;
    char *value;
    struct config_map_entry *sub;
    UT_hash_handle hh;
} config_map_entry;


typedef struct http_map_entry {
    char *key;
    char *value;
    UT_hash_handle hh;
} http_map_entry;

#endif //PROJECT6_MAP_ENTRY_H
