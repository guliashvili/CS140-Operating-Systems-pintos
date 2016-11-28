#include "mmap.h"
#include "files.h"
static bool equals_fd(const struct list_elem *elem, void *id);
static struct user_file_info *find_open_mmap(int id);

static int mmap_id = 1;

static bool equals_fd(const struct list_elem *elem, void *id){
  return list_entry (elem, struct mmap_info, link)->id == *(int*)id;
}
static struct user_file_info *find_open_mmap(int id){
  struct list_elem *e =  list_find(&thread_current()->mmap_address, equals_fd, (void*)&id);
  if(e == NULL) return NULL;
  else return list_entry(e, struct mmap_info, link);
}

int mmap(int fd, void *vaddr){
  int len = filesize_sys(fd);
  if(len == 0) return -1;
  if(len == -1) return -1;
  if(vaddr == 0) return -1;
  if(pg_round_down(vaddr) != vaddr) return -1;
  ASSERT(thread_current()->pagedir);
  void *vaddr_saved = vaddr;

  int i;
  for(i = 0; i < len; i+= PGSIZE){
    struct supp_pagedir_entry ** ee = supp_pagedir_lookup(thread_current()->supp_pagedir, vaddr + i * PGSIZE, false);
    if(ee) return -1;
  }

  for(i = 0; i < len; i += PGSIZE, vaddr += PGSIZE){
    supp_pagedir_virtual_create(vaddr, PAL_ZERO | PAL_USER);
    supp_pagedir_set_readfile(vaddr, fd, i, ((i + PGSIZE) < len) ? (i + PGSIZE) : len, false);
  }
  struct mmap_info *info = malloc(sizeof(struct mmap_info));
  info->id = mmap_id++;
  info->vaddr = vaddr_saved;
  info->num_of_pages = (len + PGSIZE - 1) / PGSIZE;

  list_push_back(&thread_current()->mmap_address, &info->link);

  return info->id;
}

void munmap(int map_id){
  struct mmap_info *info = find_open_mmap(map_id);
  if(info == NULL) exit(-1);
  int i;
  void *vaddr;
  for(i = 0, vaddr = info->vaddr; i < info->num_of_pages; i++, vaddr += PGSIZE) {
    struct supp_pagedir_entry **e = supp_pagedir_lookup(thread_current()->supp_pagedir, info->vaddr, false);
    ASSERT(e);
    ASSERT(*e);
    supp_pagedir_destroy_page(thread_current()->supp_pagedir, thread_current()->pagedir, vaddr);
  }
  list_remove(&info->link);
  free(info);
}