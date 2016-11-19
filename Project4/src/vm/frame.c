#include "vm/frame.h"
#include "lib/string.h"
#include <debug.h>

size_t frame_buf_size(int pages_cnt){
  return pages_cnt * sizeof(struct frame);
}

struct frame_map *frame_map_create_in_buf(int pages_cnt, void *base, size_t len){
  if(frame_buf_size(pages_cnt) > len)
    PANIC("wrong arguments in frame");

  memset(base, 0, len);
  return (struct frame_map *)base;
}