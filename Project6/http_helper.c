//
// Created by a on 1/19/17.
//

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
  for (int j = 0; b != '\r' || c != '\n'; a = b, b = c, assert(read(fd, &c, sizeof(c)) > 0), j++) {
    if (j >= 3) {
      if (i >= n)
        return 0;
      buffer[i++] = a;
      buffer[i] = 0;
    }
  }
  buffer[i++] = a;
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
  if (root == NULL)
    fprintf(stderr, "Nothing to read in request? WAT?\n");

  return root;
}


void http_destroy(http_map_entry *root) {
  if (root == NULL) return;

  http_map_entry *item1, *tmp1;
  HASH_ITER(hh, root, item1, tmp1) {
    printf("http key %s value %s\n", item1->key, item1->value);
    HASH_DEL(root, item1);
    free(item1->key);
    free(item1->value);
    free(item1);
  }
  printf("\n\n");
  fflush(stdout);
}

const char *http_get_val(http_map_entry *root, const char *key) {
  http_map_entry *item1 = NULL;
  HASH_FIND_STR(root, key, item1);
  if (item1) return item1->value;
  else return NULL;
}