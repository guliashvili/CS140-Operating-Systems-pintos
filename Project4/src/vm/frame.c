#include "vm/frame.h"
#include "lib/string.h"
#include <debug.h>

#include "lib/kernel/list.h"
#include "threads/synch.h"
#include "frame.h"

size_t frame_buf_size(int pages_cnt){
  return pages_cnt * sizeof(struct frame) + sizeof(struct frame_map);
}

void frame_init_single(struct frame_map *frame_map, int i, struct list_elem *link){
  if(frame_map == NULL) return;
  ASSERT(i >= 0);
  ASSERT(i < frame_map->num_of_frames);

  struct frame *f = frame_map->frames + i;
  list_init(&f->pointing_threads);
  lock_init(&f->lock);
  list_push_back(&f->pointing_threads, link);
  f->MAGIC = FRAME_MAGIC;
  f->prohibit_cache = 0;
}

bool frame_destroy_single(struct frame_map *frame_map, int i, struct list_elem *link){
  if(frame_map == NULL) {
    ASSERT(0);
  }
  ASSERT(i >= 0);
  ASSERT(i < frame_map->num_of_frames);

  struct frame *f = frame_map->frames + i;
  ASSERT(f->MAGIC == FRAME_MAGIC);
  bool ret;
  lock_acquire(&f->lock);
  list_remove(link);
  if(list_size(&f->pointing_threads) == 0){
    ret = true;
    f->MAGIC = -1;
  }else{
    ret = false;
  }
  lock_release(&f->lock);
  return ret;
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