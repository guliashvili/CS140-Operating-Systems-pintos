//
// Created by a on 1/19/17.
//
//http://tinyhttpd.sourceforge.net/
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "http_helper.h"
#include "string_helper.h"
#include "url.h"

#define BUFFER_LEN 5000

static bool read_line(int fd, char *buffer, int n) {
  char a = 0, b = 0, c = 0;
  int i = 0;
  bool could = true;
  for (int j = 0; b != '\r' || c != '\n'; a = b, b = c, could = read(fd, &c, 1), j++) {
    if(!could) {
      if(a) buffer[i++] = a;
      if(b) buffer[i++] = b;
      if(c) buffer[i++] = c;
      a = b = c = 0;
      break;
    }
    if (j >= 3) {
      if (i >= n)
        return 0;
      buffer[i++] = a;
      buffer[i] = 0;
    }
  }
  if(a) buffer[i++] = a;
  buffer[i] = 0;
  return 1;
}

static bool parse_first_line(http_map_entry **root, char *s) {
  int i = 0;
  for (char *save_ptr, *token = strtok_r(s, " ", &save_ptr); token != NULL;
       token = strtok_r(NULL, " ", &save_ptr), i++) {
    http_map_entry *current = malloc(sizeof(http_map_entry));

    current->value = calloc(1, strlen(token) * 2 + 2);
    url_decode(current->value, token);
    if (i == 0)
      current->key = strdup(HTTP_METHOD);
    else if (i == 1)
      current->key = strdup(HTTP_URI);
    else if (i == 2)
      current->key = strdup(HTTP_VERSION);
    else
      assert(0);
    HASH_ADD_STR(*root, key, current);
  }
  return i == 3;
}

static void split_range(http_map_entry **root, char *value){
  char *s = strdup(value);
  int S = 0;
  int E = INT32_MAX;

  int i = 0;
  for (char *save_ptr, *token = strtok_r(s, "= -", &save_ptr); token != NULL;
       token = strtok_r(NULL, "= -", &save_ptr), i++) {
    if(!strcmp(str_to_lower(token), "bytes")) continue;
    if(i == 1) S = atoi(token);
    else if(i == 2) E = atoi(token);
  }

  http_map_entry *current = malloc(sizeof(http_map_entry));
  current->key = strdup(HTTP_SEND_S);
  char *write = malloc(30);
  sprintf(write, "%d", S);
  current->value = write;
  HASH_ADD_STR(*root, key, current);

  current = malloc(sizeof(http_map_entry));
  current->key = strdup(HTTP_SEND_E);
  write = malloc(30);
  sprintf(write, "%d", E);
  current->value = write;
  HASH_ADD_STR(*root, key, current);

  free(s);
}
static bool parse_normal_line(http_map_entry **root, char *s) {
  int i = 0;
  http_map_entry *current = malloc(sizeof(http_map_entry));

  for (char *save_ptr, *token = strtok_r(s, ":", &save_ptr); token != NULL;
       token = strtok_r(NULL, "", &save_ptr), i++) {

    while (isspace(*token)) token++;
    int len = strlen(token);
    while (len > 0 && isspace(token[--len])) token[len] = 0;

    if (i == 0)
      current->key = str_to_lower(strdup(token));
    else if (i == 1)
      current->value = strdup(token);
    else
      assert(0);
  }
  HASH_ADD_STR(*root, key, current);

  if(strcmp(current->key, "range") == 0){
    split_range(root, current->value);
  }

  return i == 2;
}

http_map_entry *http_parse(int fd) {
  http_map_entry *root = NULL;

  char buffer[BUFFER_LEN + 1];
  for (int i = 0;; i++) {
    if (!read_line(fd, buffer, BUFFER_LEN)) {
      http_destroy(root);
      fprintf(stderr, "line was larger then the buffer\n");
      return NULL;
    }
    if (!buffer[0]) break;

    if (!i) {
      if (!parse_first_line(&root, buffer)) {
        http_destroy(root);
        fprintf(stderr, "Could not parse the first line\n");
        return NULL;
      }
    } else if (!parse_normal_line(&root, buffer)) {
      http_destroy(root);
      fprintf(stderr, "Could not parse the not first line\n");
      return NULL;
    }
  }
  http_map_entry *entry = NULL;
  HASH_FIND_STR(root, "content-length", entry);
  if (entry != NULL) {
    int len = atoi(entry->value);
    http_map_entry *current = malloc(sizeof(http_map_entry));
    current->value = malloc(len + 1);
    assert(len == read(fd, current->value, len));
    current->value[len] = 0;
    current->key = strdup(HTTP_CONTENT);
    HASH_ADD_STR(root, key, current);
  }

  if(root){
    const char *length_s;
    if(length_s = http_get_val(root, "content-length")){
      int length = atoi(length_s);
      char *con = malloc(length + 1);
      char *work = con;
      while(length > 0){
        int rd = read(fd, work, length);
        work += rd;
        length -= rd;
      }
      con[length] = 0;
      http_put_val(root, HTTP_CONTENT, con);
    }
  }

  return root;
}


void http_destroy(http_map_entry *root) {
  if (root == NULL) return;

  http_map_entry *item1, *tmp1;
  HASH_ITER(hh, root, item1, tmp1) {
    HASH_DEL(root, item1);
    free(item1->key);
    free(item1->value);
    free(item1);
  }
}

const char *http_get_val(http_map_entry *root, const char *key) {
  http_map_entry *item1 = NULL;
  HASH_FIND_STR(root, key, item1);
  if (item1) return item1->value;
  else return NULL;
}

void http_put_val(http_map_entry *root, char *key, char *value){
  http_map_entry *current = malloc(sizeof(http_map_entry));
  current->key = strdup(key);
  current->value = strdup(value);

  HASH_ADD_STR(root, key, current);
}