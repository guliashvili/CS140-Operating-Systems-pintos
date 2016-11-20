//
// Created by a on 11/20/16.
//

#include "paging.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "lib/string.h"
#include "../userprog/pagedir.h"
#include "../threads/vaddr.h"
#include "../lib/debug.h"
#include "lib/stdio.h"
#include "../threads/thread.h"

static void supp_page_destroy_local( uint32_t *pd, struct supp_pagedir_entry **v, void **p);

struct supp_page_table* init_supp_pagedir(void){
  ASSERT(PGSIZE == sizeof(struct supp_page_table));
  ASSERT(PGSIZE == sizeof(struct supp_page_tabl2));
  struct supp_page_table* ret = palloc_get_page(PAL_ZERO);

  return ret;
}

struct supp_pagedir_entry **
lookup_supp_page (struct supp_page_table *table, const void *vaddr, bool create)
{
  ASSERT (table != NULL);

  struct supp_page_tabl2 *pde = table->entries[pd_no(vaddr)];
  if(pde == NULL){
    if(create) {
      pde = table->entries[pd_no(vaddr)] = palloc_get_page(PAL_ZERO);
      ASSERT(pde);
    }else
      return NULL;
  }

  /* Return the page table entry. */
  return &pde->entries[pt_no (vaddr)];

}

bool virtually_create_page(struct supp_page_table *table, void *upage,  bool writable, enum palloc_flags flag,
                              struct file_info *file_info){

  ASSERT (pg_ofs (upage) == 0);
  ASSERT (is_user_vaddr (upage));
  ASSERT(flag & PAL_USER);

  struct supp_pagedir_entry ** elem = lookup_supp_page(table, upage, true);
  if(elem == NULL || *elem != NULL) return false;
  else{
    struct supp_pagedir_entry * el = *elem = malloc(sizeof(struct supp_pagedir_entry));

    ASSERT(table->entries[pd_no(upage)]);
    ASSERT(table->entries[pd_no(upage)]->entries[pt_no(upage)]);
    el->MAGIC = PAGING_MAGIC;
    el->flags = flag;
    el->writable = writable;
    el->pagedir = &thread_current()->pagedir;

    if(file_info == NULL){
      el->file_info.page_id = -1;
      el->file_info.path[0] = 0;
    }else{
      el->file_info.page_id = file_info->page_id;
      strlcpy(el->file_info.path, file_info->path, PATH_MAX_LENGTH);
    }
    return true;
  }

}

bool really_create_page(struct supp_page_table *table, uint32_t *pd, void *upage){
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (is_user_vaddr (upage));
  if(paging_pagedir_exists(pd, upage))
    return false;

  struct supp_pagedir_entry ** elem = lookup_supp_page(table, upage, false);
  if(elem == NULL || *elem == NULL) return false;

  struct supp_pagedir_entry * el = *elem;
  void *kpage = palloc_get_page_user_smartass(el->flags, &el->link);
  ASSERT (vtop (kpage) >> PTSHIFT < init_ram_pages);
  ASSERT (pg_ofs (kpage) == 0);

  bool ret = pagedir_set_page(pd, upage, kpage, el->writable);
  if(!ret) palloc_free_page_smartass(kpage, &el->link);
  ASSERT(paging_pagedir_exists(pd, upage));
  return ret;
}
void supp_page_destroy(struct supp_page_table *table, uint32_t *pagedir, void *upage) {
  struct supp_pagedir_entry **v = lookup_supp_page(table, upage, false);
  if(v == NULL || *v == NULL) return;
  void ** p = pagedir_get_page(pagedir, upage);
  if(p != NULL) supp_page_destroy_local(pagedir, v, p);
  else {
    palloc_free_page_smartass(*p, &(*v)->link);
    free(*v);
    *v = NULL;
  }

}

static void supp_page_destroy_local( uint32_t *pd, struct supp_pagedir_entry **v, void **p){
  palloc_free_page_smartass(pte_get_page(*p), &(*v)->link);
  free(*v);
  (*v) = NULL;
  pagedir_clear_page_given(pd, (uint32_t *)p);
}


void supp_pagedir_destroy(struct supp_page_table *table, uint32_t *pd) {
  if (table == NULL) PANIC("Something went wrong in paging destroy");
  int i, j;
  uint32_t *pde;
  for (i = 0, pde = pd; pde < pd + pd_no(PHYS_BASE); i++, pde++) {
    struct supp_page_tabl2 *ta = table->entries[i];
    if(*pde & PTE_P) {ASSERT(ta != NULL);}

    if (ta != NULL) {
      uint32_t *pt = pde_get_pt (*pde);
      uint32_t *pte;
      for (j = 0,pte = pt; pte < pt + PGSIZE / sizeof *pte;  j++, pte++) {
        if(*pte & PTE_P) {ASSERT(ta->entries[j] != NULL);}
        if (ta->entries[j] != NULL) {
          ASSERT(ta->entries[j]->MAGIC == PAGING_MAGIC);
          supp_page_destroy_local(pd, &ta->entries[j], (void**)pte);
        }
      }
      palloc_free_page(table->entries[i]);
    }
  }

  palloc_free_page(table);
}

bool paging_pagedir_exists(uint32_t *pagedir, const void *uaddr){
  return pagedir_get_page(pagedir, uaddr) != 0;
}
bool paging_supp_pagedir_exists(struct supp_page_table *pagedir, const void *uaddr){
  struct supp_pagedir_entry **r =lookup_supp_page(pagedir, uaddr, false);
  return r != NULL && *r != NULL;
}