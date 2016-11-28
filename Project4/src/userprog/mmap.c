#include "mmap.h"
#include "files.h"

int mmap(int fd, void *vaddr){
  int len = filesize_sys(fd);
  if(len == 0) return -1;
  if(len == -1) return -1;
  if(vaddr == 0) return -1;
  if(pg_round_down(vaddr) != vaddr) return -1;

  int i;
  for(i = 0; i < len; i+= PGSIZE){
    struct supp_pagedir_entry ** ee = supp_pagedir_lookup(thread_current()->supp_pagedir, vaddr + i * PGSIZE, false);
    if(ee) return -1;
  }

  for(i = 0; i < len; i += PGSIZE, vaddr += PGSIZE){
    supp_pagedir_virtual_create(vaddr, PAL_ZERO | PAL_USER);
    supp_pagedir_set_readfile(vaddr, fd, i, ((i + PGSIZE) < len) ? (i + PGSIZE) : len, false);
  }
}

void munmap(int map_id){

}