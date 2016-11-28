#include "paging.h"
#include "threads/malloc.h"
#include "threads/pte.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "frame.h"

#include "../threads/malloc.h"
#include "../lib/debug.h"
#include "../threads/pte.h"
#include "../threads/palloc.h"
#include "../threads/vaddr.h"
#include "../threads/thread.h"
#include "../userprog/pagedir.h"
#include "../userprog/syscall.h"
#include "swap.h"
#include "../filesys/filesys.h"
#include "../lib/string.h"


static bool supp_pagedir_really_create(void *upage);

/**
 * init supp pagedir internal structures
 * @return pointer to the supp pagedir struct
 */
struct supp_pagedir* supp_pagedir_init(void){
  struct supp_pagedir * ret = calloc(1, sizeof(struct supp_pagedir));
  ASSERT(ret);
  return ret;
}


void supp_pagedir_set_readfile(void *vaddr, int fd, int s, int e, bool readonly){
  struct supp_pagedir_entry ** ee = supp_pagedir_lookup(thread_current()->supp_pagedir, vaddr, false);
  ASSERT(ee);
  struct supp_pagedir_entry *el=*ee;
  ASSERT(el);
  int flags = el->flags | PAL_ZERO;
  if(readonly) flags |= PAL_READONLY;
  ASSERT(ee);
  el=*ee;
  ASSERT(el);

  el->fd = fd;
  el->s = s;
  el->e = e;
}

/**
 * Looks up for the mapping from upage to supp pagedir entry.
 * @param table  supp pagedir
 * @param upage
 * @param create should create if not exists
 * @return pointer to the pointer(of the entry).
 */
struct supp_pagedir_entry **
supp_pagedir_lookup (struct supp_pagedir *table, const void *upage, bool create)
{
  ASSERT (table);
  ASSERT(upage);
  ASSERT(pd_no(upage) < (1<<PDBITS));
  struct supp_pagedir2 *pde = table->entries[pd_no(upage)];
  if(pde == NULL){
    if(create) {
      pde = table->entries[pd_no(upage)] = calloc(1, sizeof(struct supp_pagedir2));
      ASSERT(pde);
    }else
      return NULL;
  }
  ASSERT(pt_no(upage) < (1<<PTBITS));

  /* Return the page table entry. */
  if(pde->entries[pt_no(upage)] != NULL)
    ASSERT(pde->entries[pt_no(upage)]->MAGIC == PAGING_MAGIC);
  return &pde->entries[pt_no (upage)];
}

/**
 * Makes page real. If it's not mapped to the palloc, mapps it to one. Also reocvers data if any.
 * @param f
 */
void paging_activate(void *upage){
  ASSERT(upage);
  struct supp_pagedir_entry **ff = supp_pagedir_lookup(thread_current()->supp_pagedir, upage, false);
  ASSERT(ff);
  ASSERT(*ff);

  struct supp_pagedir_entry *f = *ff;
  ASSERT(f);
  ASSERT(f->upage);

  if(!pagedir_get_page(thread_current()->pagedir, f->upage))
    supp_pagedir_really_create(f->upage);

  void *kpage = pagedir_get_page(thread_current()->pagedir, f->upage);
  ASSERT(kpage);

  if(f->sector_t != BLOCK_SECTOR_T_ERROR){
    ASSERT(f->fd == -1);
    //printf("activating %u\n",f->upage);
    swap_read(f->sector_t, f->upage);
    f->sector_t = BLOCK_SECTOR_T_ERROR;
  }else if(f->fd != -1){
    //PANIC("")
    int fd = f->fd;
    f->fd = -1;

    ASSERT(upage == pg_round_down(upage));
    seek_sys(fd, f->s);
    supp_pagedir_set_prohibit(upage, 1);
    ASSERT(f->e - f->s <= PGSIZE);
    int read_size = f->e - f->s;
    read_sys(fd, kpage, read_size);
    memset(kpage + read_size, 0, PGSIZE - read_size);
    supp_pagedir_set_prohibit(upage, 0);
    f->fd = fd; // I need it activate not to be recalled for many times
  }
}

void discard_file(struct supp_pagedir_entry *e){
  if(pagedir_is_dirty(thread_current()->pagedir, e->upage)){
    pagedir_set_dirty(thread_current()->pagedir, e->upage, false);

    seek_sys(e->fd, e->s);
    write_sys(e->fd, e->upage, e->e - e->s);
  }
}

/**
 * initializes upage in supplemental page table, but does not acquire any frame.
 * @param upage virtual user address
 * @param flag flags
 */
void supp_pagedir_virtual_create(void *upage, enum palloc_flags flag){
  ASSERT(pg_round_down(upage) == upage);
  struct supp_pagedir *table = thread_current()->supp_pagedir;
  ASSERT(table);
  ASSERT(upage);
  ASSERT(pg_ofs (upage) == 0);
  ASSERT(is_user_vaddr (upage));
  ASSERT(flag & PAL_USER);

  struct supp_pagedir_entry ** elem = supp_pagedir_lookup(table, upage, true);
  if(elem == NULL || *elem != NULL){
    PANIC("Upage %u is already used. elem = %u, *elem = %u",
          (uint32_t)upage,  (uint32_t)elem,  (uint32_t)((elem == NULL)?NULL:*elem));
  } else{
    struct supp_pagedir_entry * el = *elem = malloc(sizeof(struct supp_pagedir_entry));

    ASSERT(table->entries[pd_no(upage)]);
    ASSERT(table->entries[pd_no(upage)]->entries[pt_no(upage)]);
    el->MAGIC = PAGING_MAGIC;
    el->flags = flag;
    el->pagedir = &thread_current()->pagedir;
    el->sector_t = BLOCK_SECTOR_T_ERROR;
    el->upage = upage;
    el->s = el->e = el->fd = -1;
  }

}

/**
 * acquires frame for given user virtual adddres upage.
 * mapping should not exist in pagedir(if it does then frame is already linked to it)
 * @param upage
 * @return
 */
static bool supp_pagedir_really_create(void *upage){
  struct supp_pagedir *table = thread_current()->supp_pagedir;
  uint32_t *pd = thread_current()->pagedir;

  ASSERT(table);
  ASSERT(pd);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (is_user_vaddr (upage));
  if(pagedir_get_page(pd, upage))
    PANIC("in pagedir this upage %u should not be present",  (uint32_t)upage);

  struct supp_pagedir_entry ** elem = supp_pagedir_lookup(table, upage, false);
  if(elem == NULL || *elem == NULL) PANIC("in suppl pagedir this upage %u should exist", (uint32_t)upage);

  struct supp_pagedir_entry * el = *elem;
  ASSERT(el->upage == upage);
  void *kpage = frame_get_page(el->flags, el);

  ASSERT (vtop (kpage) >> PTSHIFT < init_ram_pages);
  ASSERT (pg_ofs (kpage) == 0);

  bool ret = pagedir_set_page(pd, upage, kpage, !(el->flags & PAL_READONLY));
  ASSERT(ret);
  ASSERT(pagedir_get_page(pd, upage) != NULL);
  return ret;
}


void supp_pagedir_destroy(struct supp_pagedir *spd, uint32_t *pd){
  int i,j;
  for(i = 0; i < (1<<PDBITS); i++){
    struct supp_pagedir2 *spd2 = spd->entries[i];
    if(spd2 == NULL) continue;
    for(j = 0; j < (1<<PTBITS); j++){
      struct supp_pagedir_entry *e = spd2->entries[j];
      if(e == NULL) continue;
      supp_pagedir_destroy_page(spd, pd, e->upage);
    }
    free(spd2);
  }
  free(spd);
}

void supp_pagedir_destroy_page(struct supp_pagedir *spd, uint32_t *pd, void *upage){
  void *kpage = pagedir_get_page(pd, upage);

  if(kpage)
    frame_free_page(kpage);

  struct supp_pagedir_entry **elem = supp_pagedir_lookup(spd, upage, false);
  pagedir_clear_page(pd, upage);

  if(elem != NULL && *elem != NULL){
    struct supp_pagedir_entry *el = *elem;
    if(el->sector_t != BLOCK_SECTOR_T_ERROR)
      swap_read(el->sector_t, NULL);
    free(el);
    (*elem) = NULL;
  }else{
    NOT_REACHED();
  }
}

void supp_pagedir_set_prohibit(void *upage, bool prohibit){
  ASSERT(upage);
  struct supp_pagedir_entry *f = *supp_pagedir_lookup(thread_current()->supp_pagedir, upage, false);
  ASSERT(f);

  if(prohibit) f->flags |= PAL_PROHIBIT_CACHE;
  else if(!prohibit) f->flags &= ~PAL_PROHIBIT_CACHE;

  lock_acquire(frame_get_lock());
  void * kpage = pagedir_get_page(thread_current()->pagedir, pg_round_down(upage));
  if(kpage) frame_set_prohibit(kpage, prohibit);
  lock_release(frame_get_lock());

  if(kpage == NULL){
    ASSERT(prohibit);
    paging_activate(upage);
    kpage = pagedir_get_page(thread_current()->pagedir, pg_round_down(upage));
  }
  ASSERT(kpage);



}