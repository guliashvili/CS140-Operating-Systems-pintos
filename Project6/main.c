#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <omp.h>
#include "uthash.h"
#include "map_entry.h"
#include "string.h"
#include "ctype.h"
#define MAX_CONFIG_LINE_L 100
static void check_gcc_version(void);
static map_entry *construct_env(int argc, char *argv[]);
static map_entry *construct_env_from_file(FILE *f);

//https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
static void check_gcc_version(void){
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 60200
  #error
  assert(0);
#endif

#undef GCC_VERSION
}

static map_entry *construct_env_from_file(FILE *f){
  map_entry *root = NULL;
  char line[MAX_CONFIG_LINE_L + 1];

  map_entry *current = malloc(sizeof(map_entry));

  current->key = NULL;
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
        current = malloc(sizeof(map_entry));
        current->key = NULL;
        current->sub = NULL;
      }
    }else{
      char *eq = strpbrk(line, "=");
      *eq = 0;
      map_entry *lvl2 = malloc(sizeof(map_entry));
      lvl2->key = strdup(line);
      lvl2->value = strdup(eq + 1);

      if(strcmp(lvl2->key, "vhost") == 0){
        if(strlen(lvl2->value) == 0){
          fprintf(stderr, "Vhost should not be empty");
          exit(0);
        }
        current->key = strdup(lvl2->key);
      }

      HASH_ADD_STR(current->sub, key, lvl2);
    }

  }
  if(current->key) HASH_ADD_STR(root, key, current);


  return root;
}

static map_entry *construct_env(int argc, char *argv[]){
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

  map_entry *ret = construct_env_from_file(f);

  fclose(f);

  return ret;
}

int main(int argc, char *argv[]) {
  check_gcc_version();
  map_entry *root = construct_env(argc, argv);


  map_entry *item1, *item2, *tmp1, *tmp2;
  HASH_ITER(hh, root, item1, tmp1) {
    printf("key %s\n",item1->key);
    HASH_ITER(hh, item1->sub, item2, tmp2) {
      printf("key = %s value = %s \n", item2->key, item2->value);
      HASH_DEL(item1->sub, item2);
      free(item2);
    }
    printf("\n\n");
    HASH_DEL(root, item1);
    free(item1);
  }
  return 0;
}