#include "vm/frame.h"
#include "lib/string.h"
#include <debug.h>

#include "lib/kernel/list.h"
#include "threads/synch.h"

size_t frame_buf_size(int pages_cnt){
  return pages_cnt * sizeof(struct frame) + sizeof(struct frame_map);
}

void frame_init_single(struct frame_map *frame_map, int i){
  if(frame_map == NULL) return;
  ASSERT(i >= 0);
  ASSERT(i < frame_map->num_of_frames);

  struct frame *f = frame_map->frames + i;
  list_init(&f->pointing_threads);
  lock_init(&f->lock);
  f->MAGIC = FRAME_MAGIC;
  f->prohibit_cache = 0;
}

void frame_init_multiple(struct frame_map *frame_map, int s,int e){
  if(frame_map == NULL) return;
  ASSERT(s < e);
  ASSERT(s >= 0);
  ASSERT(e <= frame_map->num_of_frames);

  for(; s < e; s++){
    frame_init_single(frame_map, s);
  }

}

void frame_destroy_single(struct frame_map *frame_map, int i){
  if(frame_map == NULL) return;
  ASSERT(i >= 0);
  ASSERT(i < frame_map->num_of_frames);

  struct frame *f = frame_map->frames + i;
  ASSERT(f->MAGIC == FRAME_MAGIC);

  f->MAGIC = -1;
}

void frame_destroy_multiple(struct frame_map *frame_map, int s,int e){
  if(frame_map == NULL) return;
  ASSERT(s < e);
  ASSERT(s >= 0);
  ASSERT(e <= frame_map->num_of_frames);

  for(; s <= e; s++){
    frame_init_single(frame_map, s);
  }

}

struct frame_map *frame_map_create_in_buf(int pages_cnt, void *base, size_t len){
  ASSERT(pages_cnt >= 0);
  ASSERT(len >= 0);
  if(frame_buf_size(pages_cnt) > len)
    PANIC("wrong arguments in frame");

  struct frame_map *p = (struct frame_map *)base;
  p->num_of_frames =len;
  p->frames = (struct frame*)(p + 1);

  return p;
}