//
// Created by a on 1/21/17.
//

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <omp.h>
#include <stdbool.h>
#include "uthash.h"
#include "map_entry.h"
#include "string.h"
#include "ctype.h"
#include "server.h"

#define MAX_CONFIG_LINE_L 100
static config_map_entry *ROOT = NULL;

static config_map_entry *construct_env_from_file(FILE *f){
  config_map_entry *root = NULL;
  char line[MAX_CONFIG_LINE_L + 1];

  config_map_entry *current = malloc(sizeof(config_map_entry));

  current->key = NULL;
  current->value = NULL;
  current->sub = NULL;

  while(!feof(f)){
    assert(fgets(line, MAX_CONFIG_LINE_L, f));
    int k = 0;
    for(int i = 0; line[i]; i++)
      if(!isspace(line[i]))
        line[k++] = line[i];
    line[k] = 0;

    if(strlen(line) == 0){
      if(current->key) {
        HASH_ADD_STR(root, key, current);
        current = malloc(sizeof(config_map_entry));
        current->key = NULL;
        current->value = NULL;
        current->sub = NULL;
      }
    }else{
      char *eq = strpbrk(line, "=");
      *eq = 0;
      config_map_entry *lvl2 = malloc(sizeof(config_map_entry));
      lvl2->sub = NULL;
      lvl2->key = strdup(line);
      lvl2->value = strdup(eq + 1);

      if(strcmp(lvl2->key, "vhost") == 0){
        if(strlen(lvl2->value) == 0){
          fprintf(stderr, "Vhost should not be empty");
          exit(0);
        }
        current->key = strdup(lvl2->value);
      }

      HASH_ADD_STR(current->sub, key, lvl2);
    }

  }
  if(current->key) HASH_ADD_STR(root, key, current);


  return root;
}

config_map_entry *register_config(int argc, char *argv[]){
  if(argc != 2){
    fprintf(stderr, "argc is %d", argc);
    exit(0);
  }
  char *file_name = argv[1];
  FILE *f = fopen(file_name, "r");
  if(f == NULL){
    fprintf(stderr, "Could not open config file %s", file_name);
    exit(0);
  }

  config_map_entry *ret = construct_env_from_file(f);

  fclose(f);

  return ROOT = ret;
}

void destruct_config(config_map_entry *root){
  config_map_entry *item1, *item2, *tmp1, *tmp2;
  HASH_ITER(hh, root, item1, tmp1) {
    printf("key %s\n",item1->key);
    HASH_ITER(hh, item1->sub, item2, tmp2) {
      printf("key = %s value = %s \n", item2->key, item2->value);
      HASH_DEL(item1->sub, item2);
      free(item2->key);
      free(item2->value);
      free(item2->sub);
      free(item2);
    }
    printf("\n\n");
    HASH_DEL(root, item1);
    free(item1->key);
    free(item1->value);
    free(item1->sub);
    free(item1);
  }
}

bool vhost_exists(const char *domain){
  config_map_entry *entry = NULL;
  HASH_FIND_STR(ROOT, domain, entry);
  return entry != NULL;
}
bool config_value_exists(const char *domain, const char* key){
  config_map_entry *entry = NULL, *entry2 = NULL;
  HASH_FIND_STR(ROOT, domain, entry);
  assert(entry);
  HASH_FIND_STR(entry->sub, key, entry2);
  return entry2 != NULL;
}
void *config_get_value(const char *domain, const char* key){
  config_map_entry *entry = NULL, *entry2 = NULL;
  HASH_FIND_STR(ROOT, domain, entry);
  assert(entry);
  HASH_FIND_STR(entry->sub, key, entry2);
  assert(entry2);
  return entry2->value;
}