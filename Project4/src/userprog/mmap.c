#include "mmap.h"
#include "files.h"
#include "threads/synch.h"

static bool equals_mmap_id(const struct list_elem *elem, void *id);
static bool equals_fd_id(const struct list_elem *elem, void *id);
static struct mmap_info *find_open_mmap(int id);

static int mmap_id = 1;

static bool equals_mmap_id(const struct list_elem *elem, void *id){
  return list_entry (elem, struct mmap_info, link)->id == *(int*)id;
}
static bool equals_fd_id(const struct list_elem *elem, void *id){
  return list_entry (elem, struct mmap_info, link)->fd == *(int*)id;
}
static struct mmap_info *find_open_mmap(int mmap_id){
  struct list_elem *e =  list_find(&thread_current()->mmap_address, equals_mmap_id, (void*)&mmap_id);
  if(e == NULL) return NULL;
  else return list_entry(e, struct mmap_info, link);
}

static struct mmap_info *find_open_mmap_with_fd(int fd_id){
  struct list_elem *e =  list_find(&thread_current()->mmap_address, equals_fd_id, (void*)&fd_id);
  if(e == NULL) return NULL;
  else return list_entry(e, struct mmap_info, link);
}

struct lock mmap_lock;

void mmap_init(){
  lock_init(&mmap_lock);
}

int mmap_sys(int fd, void *vaddr, int s, bool readonly){
  int len = filesize_sys(fd);
  if(len == 0) return -1;
  if(len == -1) return -1;
  if(vaddr == 0) return -1;
  if(fd < 2) return -1;
  if(pg_round_down(vaddr) != vaddr) return -1;

  ASSERT(thread_current()->pagedir);
  void *vaddr_saved = vaddr;

  int i;
  for(i = s; i < len; i+= PGSIZE){
    struct supp_pagedir_entry ** ee = supp_pagedir_lookup(thread_current()->supp_pagedir, vaddr + i * PGSIZE, false);
    if(ee) return -1;
  }

  lock_acquire(&mmap_lock);
  fd = file_reopen_sys(fd);
  ASSERT(fd != -1);
  int num_of_pages;
  for(i = s, num_of_pages = 0; i < len; i += PGSIZE, vaddr += PGSIZE, num_of_pages++){
    supp_pagedir_virtual_create(vaddr, PAL_ZERO | PAL_USER);
    supp_pagedir_set_readfile(vaddr, fd, i, ((i + PGSIZE) < len) ? (i + PGSIZE) : len, false);
  }
  struct mmap_info *info = malloc(sizeof(struct mmap_info));
  info->id = mmap_id++;
  info->vaddr = vaddr_saved;
  info->num_of_pages = num_of_pages;
  info->fd = fd;

  list_push_back(&thread_current()->mmap_address, &info->link);
  lock_release(&mmap_lock);

  return info->id;
}

void munmap_sys(int map_id){
  lock_acquire(&mmap_lock);
  struct mmap_info *info = find_open_mmap(map_id);
  ASSERT(info);
  int i;
  void *vaddr;
  for(i = 0, vaddr = info->vaddr; i < info->num_of_pages; i++, vaddr += PGSIZE) {
    struct supp_pagedir_entry **e = supp_pagedir_lookup(thread_current()->supp_pagedir, vaddr, false);
    ASSERT(e);
    ASSERT(*e);
    supp_pagedir_destroy_page(thread_current()->supp_pagedir, thread_current()->pagedir, vaddr);
  }
  list_remove(&info->link);
  free(info);
  lock_release(&mmap_lock);
}
