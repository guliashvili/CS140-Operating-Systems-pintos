//
// Created by a on 1/11/17.
//

#ifndef PROJECT6_MAP_ENTRY_H
#define PROJECT6_MAP_ENTRY_H

#include "uthash.h"
typedef struct map_entry{
    const char *key;
    const char *value;
    struct map_entry *sub;
    UT_hash_handle hh;
} map_entry;


#endif //PROJECT6_MAP_ENTRY_H
