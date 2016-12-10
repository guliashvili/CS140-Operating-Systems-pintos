#include "vm/frame.h"
#include "threads/malloc.h"
#include "../threads/malloc.h"
#include "frame.h"
#include "../lib/debug.h"
#include "../threads/palloc.h"
#include "../lib/stdint.h"
#include "../userprog/pagedir.h"
#include "../threads/thread.h"
#include "lib/random.h"
#include "../lib/random.h"
#include "paging.h"
#include "swap.h"
#include "../userprog/syscall.h"
#include "../userprog/mmap.h"

static void frame_init_single(uint32_t idx, enum palloc_flags flags, struct supp_pagedir_entry *user, void *page);
static struct frame* frame_get_frame(uint32_t idx);
static void frame_second_chance_algorithm(void);
static void add_frame_to_eviction_struct(struct frame *f);
static struct frame *get_next_frame_to_test(void);
static void remove_frame_from_eviction_struct(struct frame *f);

static struct frame_map *frame_map;

static struct frame* frame_get_frame(uint32_t idx){
  ASSERT(frame_map->num_of_frames >= 0);
  ASSERT(idx < (uint32_t)frame_map->num_of_frames);
  return frame_map->frames + idx;
}

static void add_frame_to_eviction_struct(struct frame *f){
  lock_acquire(&frame_map->list_lock);
  list_push_back(&frame_map->ordered_list, &f->link);
  lock_release(&frame_map->list_lock);
}

static struct frame *get_next_frame_to_test(void){
  lock_acquire(&frame_map->list_lock);

  struct frame *ret = list_entry(list_begin(&frame_map->ordered_list), struct frame, link);
  list_remove(&ret->link);
  list_push_back(&frame_map->ordered_list, &ret->link);

  lock_release(&frame_map->list_lock);
  return ret;
}

static void remove_frame_from_eviction_struct(struct frame *f){
  lock_acquire(&frame_map->list_lock);
  list_remove(&f->link);
  lock_release(&frame_map->list_lock);
}

/* Initializes single frame.
 * sets if its prohibited to send it in cache
 * sets pointer to the mapped supp_pagedir_entry
 * crashes if current frame is in use by someone else*/
static void frame_init_single(uint32_t idx, enum palloc_flags flags, struct supp_pagedir_entry *user, void *page){
  struct frame *f = frame_get_frame(idx);

  ASSERT(f->MAGIC != FRAME_MAGIC);
  f->MAGIC = FRAME_MAGIC;
  f->user = user;
  f->prohibit_cache = flags & PAL_PROHIBIT_CACHE;
  pagedir_set_accessed(*user->pagedir, page, false);
  pagedir_set_dirty(*user->pagedir, page, false);
  add_frame_to_eviction_struct(f);
}

/**
 * initilizes frame internal structures in the kernel pool.
 * @param pages_cnt
 */
void frame_map_init(int pages_cnt){
  static int install = 0;
  if(install++ > 1) PANIC("frame is installed more then once");
  frame_map = (struct frame_map *)malloc(sizeof(struct frame_map));
  ASSERT(frame_map);
  frame_map->frames = (struct frame *)malloc(sizeof(struct frame) * pages_cnt);
  ASSERT(frame_map->frames);
  lock_init(&frame_map->list_lock);
  list_init(&frame_map->ordered_list);
  frame_map->num_of_frames = pages_cnt;
}

/**
 * randomly moves the frame to swap(if there exists one without the prohibition)
 */
static void frame_second_chance_algorithm(void){
  struct frame *f;

  struct lock *lock;
  while(1){
    f = get_next_frame_to_test();
    ASSERT(f->MAGIC == FRAME_MAGIC);
    if(lock_try_acquire(lock = &f->user->lock)) {
      if (f->prohibit_cache) {
        lock_release(lock);
      } else{
        if(pagedir_is_accessed(*f->user->pagedir, f->user->upage)
           || pagedir_is_accessed(*f->user->pagedir,
                                  pagedir_get_page(*f->user->pagedir, f->user->upage))){
          pagedir_set_accessed(*f->user->pagedir, f->user->upage, false);
          pagedir_set_accessed(*f->user->pagedir,
                               pagedir_get_page(*f->user->pagedir, f->user->upage), false);
          lock_release(lock);
        }else
          break;
      }
    }else{
      //its being access so in any way accessed would be one now.
    }

  }

  /**
   * deadlock is not gonna happen. I'm not locking in order,
   * but first lock acquired in exception is lock on the supp_pagedir_entry without entry,
   * second lock is on the supp_pagedir_entry with frame. ( so they will not make deadlock )
   */


  ASSERT(*f->user->pagedir);
  void *kpage = pagedir_get_page(*f->user->pagedir, f->user->upage);
  ASSERT(kpage);
  ASSERT(pg_round_down(kpage) == kpage);
  ASSERT(f);
  ASSERT(f->user);
  ASSERT(f->user->pagedir);
  ASSERT(f->user->upage);
  ASSERT(f->user->sector_t == -1);

  struct supp_pagedir_entry *user = f->user;

  uint32_t* pagedir_save = thread_current()->pagedir;
  pagedir_activate(*user->pagedir);
  pageir_set_kernel_access(*user->pagedir, user->upage, true);

  if(user->fd != -1 && mmap_discard(user)){

  }else {
    if(user->fd == -1 && (user->flags & PAL_ZERO) &&
       !pagedir_is_dirty(*user->pagedir, user->upage) &&
       !pagedir_is_dirty(*user->pagedir, kpage)){

    }else{
      f->user->sector_t = swap_write(kpage);
    }
  }

  pagedir_clear_page(*user->pagedir, user->upage);
  pagedir_activate(pagedir_save);
  frame_free_page_no_lock(kpage);
  ASSERT(*user->pagedir != NULL);

  lock_release(lock);
}

/**
 * allocates  frame, if no free slot available moves one in swap
 * @param flags
 * @param user
 * @return
 */
void* frame_get_page(enum palloc_flags flags, struct supp_pagedir_entry *user) {
  ASSERT(user->lock.holder == thread_current());
  ASSERT(!(flags & PAL_THROUGH_FRAME));
  ASSERT(flags & PAL_USER);

  flags |= PAL_THROUGH_FRAME;

  void *page = palloc_get_page(flags);

  while(page == NULL){ // no free page available
    frame_second_chance_algorithm();

    page = palloc_get_page(flags);

  }

  uint32_t idx = palloc_page_to_idx(flags, page);

  frame_init_single(idx, flags, user, page);


  return page;
}

void frame_free_page_no_lock (void *kpage){
  uint32_t idx = palloc_page_to_idx(PAL_USER | PAL_THROUGH_FRAME, kpage);
  ASSERT(idx != UINT32_MAX);
  struct frame *f = frame_get_frame(idx);
  remove_frame_from_eviction_struct(f);
  f->MAGIC = -1;
  f->user = NULL;

  palloc_free_page(kpage);
}

void frame_set_prohibit(void *kpage, bool prohibit){
  uint32_t idx = palloc_page_to_idx(PAL_USER | PAL_THROUGH_FRAME, kpage);
  ASSERT(idx != UINT32_MAX);
  struct frame *f = frame_get_frame(idx);
  ASSERT(f);
  f->prohibit_cache = prohibit;
}
