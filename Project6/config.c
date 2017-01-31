//
// Created by a on 1/21/17.
//

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "ctype.h"

#define MAX_CONFIG_LINE_L 300
static config_map_entry *ROOT = NULL;

/*
 * installs lock for each log file
 */
static void add_lock(const char *key, config_map_entry *current){

  config_map_entry *lvl2 = malloc(sizeof(config_map_entry));
  lvl2->sub = NULL;
  lvl2->key = strdup(key);
  lvl2->value = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init((pthread_mutex_t*)lvl2->value, NULL);

  HASH_ADD_STR(current->sub, key, lvl2);
}

/*
 * constructs global map from opened file
 */
static config_map_entry *construct_env_from_file(FILE *f) {
  config_map_entry *root = NULL;
  char line[MAX_CONFIG_LINE_L + 1];

  config_map_entry *current = calloc(1, sizeof(config_map_entry));

  while (!feof(f)) {
    if(!fgets(line, MAX_CONFIG_LINE_L, f) && feof(f)) break;

    int k = 0;
    for (int i = 0; line[i]; i++)
      if (!isspace(line[i]))
        line[k++] = line[i];
    line[k] = 0;

    if (strlen(line) == 0) {
      if (current->key) {
        add_lock(LOG_INFO_KEY, current);
        HASH_ADD_STR(root, key, current);
        current = calloc(1, sizeof(config_map_entry));
      }
    } else {
      char *eq = strpbrk(line, "=");
      *eq = 0;
      config_map_entry *lvl2 = malloc(sizeof(config_map_entry));
      lvl2->sub = NULL;
      lvl2->key = strdup(line);
      lvl2->value = strdup(eq + 1);

      if (strcmp(lvl2->key, "vhost") == 0) {
        if (strlen(lvl2->value) == 0) {
          fprintf(stderr, "Vhost should not be empty");
          exit(0);
        }
        current->key = strdup(lvl2->value);
      }
      HASH_ADD_STR(current->sub, key, lvl2);
    }

  }
  if (current->key) {
    add_lock(LOG_INFO_KEY, current);
    HASH_ADD_STR(root, key, current);
  } else
    free(current);

  return root;
}

/*
 * registers global config map using arguments passed to main
 */
config_map_entry *register_config(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "argc is %d", argc);
    exit(0);
  }
  char *file_name = argv[1];
  FILE *f = fopen(file_name, "r");
  if (f == NULL) {
    fprintf(stderr, "Could not open config file %s", file_name);
    exit(0);
  }

  config_map_entry *ret = construct_env_from_file(f);

  fclose(f);

  return ROOT = ret;
}

/*
 * It's never called, server does not support shutdown
 */
void destruct_config(config_map_entry *root) {
  config_map_entry *item1, *item2, *tmp1, *tmp2;
  HASH_ITER(hh, root, item1, tmp1) {
    HASH_ITER(hh, item1->sub, item2, tmp2) {
      HASH_DEL(item1->sub, item2);
      free(item2->key);
      free(item2->value);
      free(item2->sub);
      free(item2);
    }
    HASH_DEL(root, item1);
    free(item1->key);
    free(item1->value);
    free(item1->sub);
    free(item1);
  }
}

/*
 * returns true if map[domain] exists
 */
bool vhost_exists(const char *domain) {
  config_map_entry *entry = NULL;
  HASH_FIND_STR(ROOT, domain, entry);
  return entry != NULL;
}

/*
 * returns true if value exists in map[domain][key]
 */
bool config_value_exists(const char *domain, const char *key) {
  config_map_entry *entry = NULL, *entry2 = NULL;
  HASH_FIND_STR(ROOT, domain, entry);
  assert(entry);
  HASH_FIND_STR(entry->sub, key, entry2);
  return entry2 != NULL;
}

/*
 * returns value stored in map[domain][key]
 */
void *config_get_value(const char *domain, const char *key) {
  assert(domain);
  assert(key);
  config_map_entry *entry = NULL, *entry2 = NULL;
  HASH_FIND_STR(ROOT, domain, entry);
  assert(entry);
  HASH_FIND_STR(entry->sub, key, entry2);
  assert(entry2);
  return entry2->value;
}