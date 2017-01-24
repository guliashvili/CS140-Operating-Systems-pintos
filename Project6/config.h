//
// Created by a on 1/21/17.
//

#ifndef PROJECT6_CONFIG_H
#define PROJECT6_CONFIG_H

#include <stdbool.h>
#include "map_entry.h"

config_map_entry *register_config(int argc, char *argv[]);

void destruct_config(config_map_entry *root);

bool vhost_exists(const char *domain);

bool config_value_exists(const char *domain, const char *key);

void *config_get_value(const char *domain, const char *key);

#endif //PROJECT6_CONFIG_H
