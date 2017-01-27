//
// Created by a on 1/21/17.
//

#ifndef PROJECT6_CONFIG_H
#define PROJECT6_CONFIG_H

#include <stdbool.h>
#include "map_entry.h"
#define LOG_INFO_KEY "log_info_lock_key"
/*
 * registers global config map using arguments passed to main
 */
config_map_entry *register_config(int argc, char *argv[]);
/*
 * It's never called, server does not support shutdown
 */
void destruct_config(config_map_entry *root);
/*
 * returns true if map[domain] exists
 */
bool vhost_exists(const char *domain);
/*
 * returns true if value exists in map[domain][key]
 */
bool config_value_exists(const char *domain, const char *key);
/*
 * returns value stored in map[domain][key]
 */
void *config_get_value(const char *domain, const char *key);

#endif //PROJECT6_CONFIG_H
