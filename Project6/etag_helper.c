//
// Created by a on 1/26/17.
//


#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "string_helper.h"

#define ETAG_CHUNK 20

static int po;
static char *printables;

void etag_init(){
  int c = 0;
  for(int i = 0; i < 255; i++)
    c += !!isgraph(i);
  po = 0;
  for(; c; c>>=1, po++);
  po--;
  c = 1<<po;
  printables = malloc(c);
  for(int i = 0; i < 255 && c; i++){
    if(isgraph(i))
      printables[--c] = i;
  }
}

static void add_hash(bool hash[8 * ETAG_CHUNK], const char *buf, int size){
  for(int i = 0; i < size; i++)
    for(int j = 0; j < 8; j++){
      int ind = i * 8 + j;
      bool ki = buf[i] & (1U<<j);
      hash[ind] ^= ki;
    }
}

static void transform(char *printable, int size, const bool hash[8 * ETAG_CHUNK]){
  memset(printable, '0', size);

  for(int i = 0, cur = 0, active = 0; cur < size; i++){
    if(i < 8 * ETAG_CHUNK)
      active |= hash[i] * (1<<(i % po));
    if(i % po == po - 1){
      printable[cur++] = printables[active];
      active = 0;
    }
  }

}

void etag_generate_str(char *buf, int buf_size, char *in, int size){
  buf_size -= 3;
  assert(buf_size > 0);

  buf[0] = '\"';
  buf[buf_size + 2] = 0;
  buf[buf_size + 1] = '\"';
  buf++;

  bool hash[8 * ETAG_CHUNK];
  memset(hash, 0, sizeof(hash));

  for(int i = 0; i < size; i += ETAG_CHUNK){
    add_hash(hash, in, MIN(ETAG_CHUNK, size - i));
  }

  transform(buf, buf_size, hash);
}

void etag_generate(char *buf, int buf_size, int file_fd){
  buf_size -= 3;
  assert(buf_size > 0);

  buf[0] = '\"';
  buf[buf_size + 2] = 0;
  buf[buf_size + 1] = '\"';
  buf++;

  lseek(file_fd, 0, SEEK_SET);

  char tmp[ETAG_CHUNK];
  bool hash[8 * ETAG_CHUNK];
  memset(hash, 0, sizeof(hash));

  int red;
  while((red = read(file_fd, tmp, ETAG_CHUNK)) > 0){
    add_hash(hash, tmp, red);
  }

  transform(buf, buf_size, hash);
  lseek(file_fd, 0, SEEK_SET);

}
