#include "paging.h"
#include "threads/malloc.h"
#include "threads/pte.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "frame.h"

#include "../threads/malloc.h"
#include "../lib/debug.h"
#include "../threads/pte.h"
#include "../threads/palloc.h"
#include "../threads/vaddr.h"
#include "../threads/thread.h"
#include "../userprog/pagedir.h"
#include "swap.h"

struct supp_pagedir* supp_pagedir_init(void){
  struct supp_pagedir * ret = calloc(1, sizeof(struct supp_pagedir));
  ASSERT(ret);
  return ret;
}

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

void paging_activate(struct supp_pagedir_entry *f){
  if(f->sector_t != BLOCK_SECTOR_T_ERROR){
    ASSERT(pg_round_down(f->upage) == f->upage);
    swap_read(f->sector_t, pagedir_get_page(*f->pagedir, f->upage));
    f->sector_t = BLOCK_SECTOR_T_ERROR;
  }
}

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
  }

}

bool supp_pagedir_really_create(void *upage){
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
  void * kpage = pagedir_get_page(thread_current()->pagedir, pg_round_down(upage));
  ASSERT(kpage);

  struct supp_pagedir_entry *f = *supp_pagedir_lookup(thread_current()->supp_pagedir, upage, false);
  ASSERT(f);

  if(prohibit && !(f->flags & PAL_PROHIBIT_CACHE)) f->flags |= PAL_PROHIBIT_CACHE;
  else if(!prohibit && (f->flags & PAL_PROHIBIT_CACHE)) f->flags ^= PAL_PROHIBIT_CACHE;

  frame_set_prohibit(kpage, prohibit);

}