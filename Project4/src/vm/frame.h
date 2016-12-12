#ifndef PROJECT4_FRAME_H
#define PROJECT4_FRAME_H

#include "threads/synch.h"
#include "../threads/synch.h"
#include "threads/palloc.h"
#include "lib/kernel/list.h"

#define FRAME_MAGIC 432432232

struct frame{
    struct supp_pagedir_entry *user;
    bool prohibit_cache;
    struct list_elem link;
};
struct frame_map{
    int num_of_frames;
    struct frame *frames;

    struct lock list_lock;
    struct list ordered_list;
};
int frame_map_get_init_size(int pages_cnt);
void frame_map_init(void *p, int pages_cnt);
void * frame_get_page(enum palloc_flags flags, struct supp_pagedir_entry *user);
void frame_free_page_no_lock (void *kpage);
void frame_set_prohibit(void *kpage, bool prohibit);
#endif //PROJECT4_FRAME_H
